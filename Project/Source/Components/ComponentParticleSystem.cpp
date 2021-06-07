#include "ComponentParticleSystem.h"

#include "GameObject.h"
#include "Components/ComponentTransform.h"
#include "Components/UI/ComponentButton.h"
#include "Application.h"
#include "Modules/ModulePrograms.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModuleRender.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleTime.h"
#include "Modules/ModuleUserInterface.h"
#include "Panels/PanelScene.h"
#include "Resources/ResourceTexture.h"
#include "Resources/ResourceShader.h"
#include "FileSystem/TextureImporter.h"
#include "FileSystem/JsonValue.h"
#include "Utils/Logging.h"

#include "Math/float3x3.h"
#include "Utils/ImGuiUtils.h"
#include "Math/TransformOps.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "GL/glew.h"
#include "debugdraw.h"
#include <random>

#include "Utils/Leaks.h"

#define JSON_TAG_EMITTER_TYPE "EmitterType"
#define JSON_TAG_BILLBOARD_TYPE "BillboardType"
#define JSON_TAG_TEXTURE_TEXTURE_ID "TextureId"

#define JSON_TAG_IS_PLAYING "IsPlaying"
#define JSON_TAG_LOOPING "IsLooping"
#define JSON_TAG_SIZE_OVER_TIME "IsSizeOverTime"
#define JSON_TAG_SCALE_FACTOR "ScaleFactor"
#define JSON_TAG_SCALE_PARTICLE "ParticleScale"
#define JSON_TAG_MAX_PARTICLE "MaxParticle"
#define JSON_TAG_VELOCITY "Velocity"
#define JSON_TAG_LIFE "LifeParticle"
#define JSON_TAG_TEXTURE_DISTANCE_REVERSE "DistanceReverse"
#define JSON_TAG_TEXTURE_REVERSE_EFFECT "ReverseEffect"
#define JSON_TAG_START_DELAY_TIME "StartDelay"

#define JSON_TAG_YTILES "Ytiles"
#define JSON_TAG_XTILES "Xtiles"
#define JSON_TAG_ANIMATION_SPEED "AnimationSpeed"
#define JSON_TAG_IS_RANDOM_FRAME "IsRandomFrame"

#define JSON_TAG_INIT_COLOR "InitColor"
#define JSON_TAG_FINAL_COLOR "FinalColor"
#define JSON_TAG_START_TRANSITION "StartTransition"
#define JSON_TAG_END_TRANSITION "EndTransition"

#define JSON_TAG_FLIP_TEXTURE "FlipTexture"

#define JSON_TAG_CONE_RADIUS_UP "ConeRadiusUp"
#define JSON_TAG_CONE_RADIUS_DOWN "ConeRadiusDown"

void ComponentParticleSystem::OnEditorUpdate() {
	if (ImGui::Checkbox("Active", &active)) {
		if (GetOwner().IsActive()) {
			if (active) {
				Enable();
			} else {
				Disable();
			}
		}
	}
	ImGui::Separator();

	ImGui::TextColored(App->editor->textColor, "Texture Settings");

	ImGui::Checkbox("Loop", &looping);
	if (ImGui::Button("Play")) Play();
	if (isPlaying) {
		ImGui::SameLine();
		ImGui::TextColored(App->editor->textColor, "Is Playing");
	}
	if (ImGui::Button("Stop")) Stop();

	ImGui::Separator();
	if (ImGui::DragFloat("Start Delay", &startDelay, App->editor->dragSpeed2f, 0, inf)) {
		restDelayTime = startDelay;
	}
	if (startDelay > 0) ImGui::DragFloat("Rest Time", &restDelayTime, App->editor->dragSpeed2f, 0, inf, "%.3f", ImGuiSliderFlags_NoInput);
	ImGui::Separator();
	const char* billboardTypeCombo[] = {"LookAt", "Stretch", "Horitzontal"};
	const char* billboardTypeComboCurrent = billboardTypeCombo[(int) billboardType];
	ImGui::TextColored(App->editor->textColor, "Type:");
	if (ImGui::BeginCombo("##Type", billboardTypeComboCurrent)) {
		for (int n = 0; n < IM_ARRAYSIZE(billboardTypeCombo); ++n) {
			bool isSelected = (billboardTypeComboCurrent == billboardTypeCombo[n]);
			if (ImGui::Selectable(billboardTypeCombo[n], isSelected)) {
				billboardType = (BillboardType) n;
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::Separator();
	const char* emitterTypeCombo[] = {"Cone", "Sphere", "Hemisphere", "Donut", "Circle", "Rectangle"};
	const char* emitterTypeComboCurrent = emitterTypeCombo[(int) emitterType];
	ImGui::TextColored(App->editor->textColor, "Shape");
	if (ImGui::BeginCombo("##Shape", emitterTypeComboCurrent)) {
		for (int n = 0; n < IM_ARRAYSIZE(emitterTypeCombo); ++n) {
			bool isSelected = (emitterTypeComboCurrent == emitterTypeCombo[n]);
			if (ImGui::Selectable(emitterTypeCombo[n], isSelected)) {
				emitterType = (EmitterType) n;
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if (emitterType == EmitterType::CONE) {
		ImGui::DragFloat("Radius Up", &coneRadiusUp, App->editor->dragSpeed2f, 0, inf);
		ImGui::DragFloat("Radius Down", &coneRadiusDown, App->editor->dragSpeed2f, 0, inf);
	}
	ImGui::Checkbox("Reverse Effect", &reverseEffect);
	if (reverseEffect) {
		ImGui::DragFloat("Distance", &distanceReverse, App->editor->dragSpeed2f, 0, inf);
	}
	UID oldID = textureID;
	ImGui::ResourceSlot<ResourceTexture>("texture", &textureID);

	ResourceTexture* textureResource = App->resources->GetResource<ResourceTexture>(textureID);

	if (textureResource != nullptr) {
		int width;
		int height;
		glGetTextureLevelParameteriv(textureResource->glTexture, 0, GL_TEXTURE_WIDTH, &width);
		glGetTextureLevelParameteriv(textureResource->glTexture, 0, GL_TEXTURE_HEIGHT, &height);

		if (oldID != textureID) {
			ComponentTransform2D* transform2D = GetOwner().GetComponent<ComponentTransform2D>();
			if (transform2D != nullptr) {
				transform2D->SetSize(float2(static_cast<float>(width), static_cast<float>(height)));
			}
		}

		ImGui::NewLine();
		ImGui::Separator();
		ImGui::TextColored(App->editor->titleColor, "Texture Preview");
		ImGui::TextWrapped("Size:");
		ImGui::SameLine();
		ImGui::TextWrapped("%i x %i", width, height);
		ImGui::Image((void*) textureResource->glTexture, ImVec2(200, 200));
		ImGui::Separator();

		ImGui::NewLine();
		ImGui::TextColored(App->editor->titleColor, "Texture Sheet Animation");
		ImGui::DragScalar("Xtiles", ImGuiDataType_U32, &Xtiles);
		ImGui::DragScalar("Ytiles", ImGuiDataType_U32, &Ytiles);
		ImGui::DragFloat("Animation Speed", &animationSpeed, App->editor->dragSpeed2f, -inf, inf);
		ImGui::Checkbox("Random Frame", &isRandomFrame);

		ImGui::NewLine();
		ImGui::DragFloat("Scale", &scale, App->editor->dragSpeed2f, 0, inf);
		ImGui::DragFloat("Life", &particleLife, App->editor->dragSpeed2f, 0, inf);

		if (ImGui::DragFloat("Speed", &velocity, App->editor->dragSpeed2f, 0, inf)) {
			CreateParticles(maxParticles, velocity);
		}

		if (ImGui::DragScalar("MaxParticles", ImGuiDataType_U32, &maxParticles)) {
			if (maxParticles <= 2000) {
				CreateParticles(maxParticles, velocity);
			} else {
				LOG("Warning: Max particles: 2000")
			}
		}

		ImGui::Checkbox("Size Over Time", &sizeOverTime);
		if (sizeOverTime) {
			ImGui::DragFloat("Scale Factor", &scaleFactor, App->editor->dragSpeed2f, -inf, inf);
		}

		ImGui::NewLine();
		ImGui::BeginColumns("##color_gradient", 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder | ImGuiColumnsFlags_NoPreserveWidths | ImGuiOldColumnFlags_NoForceWithinWindow);
		{
			ImGui::ColorEdit4("Init Color", initC.ptr(), ImGuiColorEditFlags_NoInputs);
			ImGui::ColorEdit4("Final Color", finalC.ptr(), ImGuiColorEditFlags_NoInputs);
		}
		ImGui::NextColumn();
		{
			ImGui::SliderFloat("Start##start_transition", &startTransition, 0.0f, endTransition, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::SliderFloat("End##end_transition", &endTransition, startTransition, particleLife, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		}
		ImGui::EndColumns();

		ImGui::NewLine();
		ImGui::Text("Flip: ");
		ImGui::SameLine();
		ImGui::Checkbox("X", &flipTexture[0]);
		ImGui::SameLine();
		ImGui::Checkbox("Y", &flipTexture[1]);
	}
}

float3 ComponentParticleSystem::CreateDirection() {
	float x, y, z;
	if (emitterType == EmitterType::CONE) {
		ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();
		float3 forward = transform->GetGlobalRotation() * float3::unitY;
		if (reverseEffect) return forward.Normalized();

		x = (float(rand()) / float((RAND_MAX)) * 0.2f) - 0.2f;
		y = (float(rand()) / float((RAND_MAX)) * 0.5f) - 0.0f;
		z = (float(rand()) / float((RAND_MAX)) * 0.5f) - 0.2f;

		return float3(forward.x + x, forward.y + y, forward.z + z).Normalized();
	}

	if (emitterType == EmitterType::SPHERE) {
		ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();
		x = (float(rand()) / float((RAND_MAX)) * 2.0f) - 1.0f;
		y = (float(rand()) / float((RAND_MAX)) * 2.0f) - 1.0f;
		z = (float(rand()) / float((RAND_MAX)) * 2.0f) - 1.0f;
		return float3(x, y, z);
	}

	return float3(0, 0, 0);
};

float3 ComponentParticleSystem::CreatePosition() {
	float x, y, z;
	if (emitterType == EmitterType::CONE) {
		ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();
		if (reverseEffect) {
			float3 forward = transform->GetGlobalRotation() * float3::unitY;
			x = (transform->GetGlobalPosition().x) + (float(rand()) / float((RAND_MAX)) * coneRadiusUp * 2) - coneRadiusUp;
			y = (transform->GetGlobalPosition().y) + (float(rand()) / float((RAND_MAX)) * coneRadiusUp) - 0.0f;
			z = (transform->GetGlobalPosition().z) + (float(rand()) / float((RAND_MAX)) * coneRadiusUp * 2) - coneRadiusUp;
			return (forward.Normalized() * distanceReverse) + (float3(x, y, z));
		} else {
			x = (transform->GetGlobalPosition().x) + (float(rand()) / float((RAND_MAX)) * coneRadiusDown * 2) - coneRadiusDown;
			y = (transform->GetGlobalPosition().y) + (float(rand()) / float((RAND_MAX)) * coneRadiusDown) - 0.0f;
			z = (transform->GetGlobalPosition().z) + (float(rand()) / float((RAND_MAX)) * coneRadiusDown * 2) - coneRadiusDown;
		}
		return (float3(x, y, z));
	}

	if (emitterType == EmitterType::SPHERE) {
		ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();
		if (reverseEffect) {
			x = (transform->GetGlobalPosition().x) + (float(rand()) / float((RAND_MAX)) * distanceReverse) - (distanceReverse / 2);
			z = (transform->GetGlobalPosition().z) + (float(rand()) / float((RAND_MAX)) * distanceReverse) - (distanceReverse / 2);
			y = (transform->GetGlobalPosition().y) + (float(rand()) / float((RAND_MAX)) * distanceReverse) - (distanceReverse / 2);
		} else {
			x = (transform->GetGlobalPosition().x);
			z = (transform->GetGlobalPosition().z);
			y = (transform->GetGlobalPosition().y);
		}
		return (float3(x, y, z));
	}

	return float3(0, 0, 0);
}

void ComponentParticleSystem::CreateParticles(unsigned nParticles, float vel) {
	particles.Allocate(maxParticles);
	for (Particle& currentParticle : particles) {
		currentParticle.scale = float3(0.1f, 0.1f, 0.1f) * scale;
		currentParticle.initialPosition = CreatePosition();
		currentParticle.position = currentParticle.initialPosition;
		currentParticle.direction = CreateDirection();
		currentParticle.velocity = vel;
	}
}

void ComponentParticleSystem::Load(JsonValue jComponent) {
	emitterType = (EmitterType)(int) jComponent[JSON_TAG_EMITTER_TYPE];
	billboardType = (BillboardType)(int) jComponent[JSON_TAG_BILLBOARD_TYPE];

	textureID = jComponent[JSON_TAG_TEXTURE_TEXTURE_ID];
	if (textureID != 0) {
		App->resources->IncreaseReferenceCount(textureID);
	}

	isPlaying = jComponent[JSON_TAG_IS_PLAYING];
	looping = jComponent[JSON_TAG_LOOPING];
	sizeOverTime = jComponent[JSON_TAG_SIZE_OVER_TIME];
	scaleFactor = jComponent[JSON_TAG_SCALE_FACTOR];
	scale = jComponent[JSON_TAG_SCALE_PARTICLE];
	maxParticles = jComponent[JSON_TAG_MAX_PARTICLE];
	velocity = jComponent[JSON_TAG_VELOCITY];
	particleLife = jComponent[JSON_TAG_LIFE];

	distanceReverse = jComponent[JSON_TAG_TEXTURE_DISTANCE_REVERSE];
	reverseEffect = jComponent[JSON_TAG_TEXTURE_REVERSE_EFFECT];
	startDelay = jComponent[JSON_TAG_START_DELAY_TIME];

	Ytiles = jComponent[JSON_TAG_YTILES];
	Xtiles = jComponent[JSON_TAG_XTILES];
	animationSpeed = jComponent[JSON_TAG_ANIMATION_SPEED];
	isRandomFrame = jComponent[JSON_TAG_IS_RANDOM_FRAME];

	JsonValue jColor = jComponent[JSON_TAG_INIT_COLOR];
	initC.Set(jColor[0], jColor[1], jColor[2], jColor[3]);
	JsonValue jColor2 = jComponent[JSON_TAG_FINAL_COLOR];
	finalC.Set(jColor2[0], jColor2[1], jColor2[2], jColor[3]);
	startTransition = jComponent[JSON_TAG_START_TRANSITION];
	endTransition = jComponent[JSON_TAG_END_TRANSITION];

	JsonValue jFlip = jComponent[JSON_TAG_FLIP_TEXTURE];
	flipTexture[0] = jFlip[0];
	flipTexture[1] = jFlip[1];

	coneRadiusUp = jComponent[JSON_TAG_CONE_RADIUS_UP];
	coneRadiusDown = jComponent[JSON_TAG_CONE_RADIUS_DOWN];

	particleSpawned = 0;
	CreateParticles(maxParticles, velocity);
}

void ComponentParticleSystem::Save(JsonValue jComponent) const {
	jComponent[JSON_TAG_EMITTER_TYPE] = (int) emitterType;
	jComponent[JSON_TAG_BILLBOARD_TYPE] = (int) billboardType;

	jComponent[JSON_TAG_TEXTURE_TEXTURE_ID] = textureID;

	jComponent[JSON_TAG_IS_PLAYING] = isPlaying;
	jComponent[JSON_TAG_LOOPING] = looping;
	jComponent[JSON_TAG_SIZE_OVER_TIME] = sizeOverTime;
	jComponent[JSON_TAG_SCALE_FACTOR] = scaleFactor;
	jComponent[JSON_TAG_SCALE_PARTICLE] = scale;
	jComponent[JSON_TAG_MAX_PARTICLE] = maxParticles;
	jComponent[JSON_TAG_VELOCITY] = velocity;
	jComponent[JSON_TAG_LIFE] = particleLife;
	jComponent[JSON_TAG_TEXTURE_DISTANCE_REVERSE] = distanceReverse;
	jComponent[JSON_TAG_TEXTURE_REVERSE_EFFECT] = reverseEffect;
	jComponent[JSON_TAG_START_DELAY_TIME] = startDelay;

	jComponent[JSON_TAG_YTILES] = Ytiles;
	jComponent[JSON_TAG_XTILES] = Xtiles;
	jComponent[JSON_TAG_ANIMATION_SPEED] = animationSpeed;
	jComponent[JSON_TAG_IS_RANDOM_FRAME] = isRandomFrame;

	JsonValue jColor = jComponent[JSON_TAG_INIT_COLOR];
	jColor[0] = initC.x;
	jColor[1] = initC.y;
	jColor[2] = initC.z;
	jColor[3] = initC.w;
	JsonValue jColor2 = jComponent[JSON_TAG_FINAL_COLOR];
	jColor2[0] = finalC.x;
	jColor2[1] = finalC.y;
	jColor2[2] = finalC.z;
	jColor2[3] = finalC.w;
	jComponent[JSON_TAG_START_TRANSITION] = startTransition;
	jComponent[JSON_TAG_END_TRANSITION] = endTransition;

	JsonValue jFlip = jComponent[JSON_TAG_FLIP_TEXTURE];
	jFlip[0] = flipTexture[0];
	jFlip[1] = flipTexture[1];

	jComponent[JSON_TAG_CONE_RADIUS_UP] = coneRadiusUp;
	jComponent[JSON_TAG_CONE_RADIUS_DOWN] = coneRadiusDown;
}

void ComponentParticleSystem::Update() {
	deadParticles.clear();
	if (restDelayTime <= 0) {
		for (Particle& currentParticle : particles) {
			if (executer) {
				currentParticle.life = -1;
			} else {
				UpdateVelocity(&currentParticle);
				if (billboardType == BillboardType::LOOK_AT) {
					currentParticle.model = float4x4::FromTRS(currentParticle.position, currentParticle.rotation, currentParticle.scale);
				} else {
					currentParticle.modelStretch.SetTranslatePart(currentParticle.position);
				}

				UpdateLife(&currentParticle);
				if (sizeOverTime) {
					UpdateScale(&currentParticle);
				}
			}
			if (currentParticle.life < 0) {
				deadParticles.push_back(&currentParticle);
			}
		}
		if (executer) executer = false;
		UndertakerParticle();
	} else {
		if (!isPlaying) return;
		if (App->time->IsGameRunning()) {
			restDelayTime -= App->time->GetDeltaTime();
		} else {
			restDelayTime -= App->time->GetRealTimeDeltaTime();
		}
	}
}

void ComponentParticleSystem::UndertakerParticle() {
	for (Particle* currentParticle : deadParticles) {
		particles.Release(currentParticle);
	}

	if (looping || (particleSpawned < maxParticles)) {
		SpawnParticle();
	} else {
		if (particles.Count() == 0) {
			restDelayTime = startDelay;
			isPlaying = false;
		}
	}
}

void ComponentParticleSystem::UpdateVelocity(Particle* currentParticle) {
	if (App->time->IsGameRunning()) {
		if (reverseEffect) {
			ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();
			float3 direction = currentParticle->position - transform->GetGlobalPosition();
			currentParticle->position -= direction * velocity * App->time->GetDeltaTime();
		} else {
			currentParticle->position += currentParticle->direction * velocity * App->time->GetDeltaTime();
		}
	} else {
		if (reverseEffect) {
			ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();
			float3 direction = currentParticle->position - transform->GetGlobalPosition();
			currentParticle->position -= direction * velocity * App->time->GetRealTimeDeltaTime();
		} else {
			currentParticle->position += currentParticle->direction * velocity * App->time->GetRealTimeDeltaTime();
		}
	}
}
void ComponentParticleSystem::UpdateScale(Particle* currentParticle) {
	if (App->time->IsGameRunning()) {
		currentParticle->scale.x += scaleFactor * App->time->GetDeltaTime();
		currentParticle->scale.y += scaleFactor * App->time->GetDeltaTime();
		currentParticle->scale.z += scaleFactor * App->time->GetDeltaTime();
	} else {
		currentParticle->scale.x += scaleFactor * App->time->GetRealTimeDeltaTime();
		currentParticle->scale.y += scaleFactor * App->time->GetRealTimeDeltaTime();
		currentParticle->scale.z += scaleFactor * App->time->GetRealTimeDeltaTime();
	}
	if (currentParticle->scale.x < 0) {
		currentParticle->scale.x = 0;
	}
	if (currentParticle->scale.y < 0) {
		currentParticle->scale.y = 0;
	}
	if (currentParticle->scale.z < 0) {
		currentParticle->scale.z = 0;
	}
}
void ComponentParticleSystem::UpdateLife(Particle* currentParticle) {
	if (App->time->IsGameRunning()) {
		currentParticle->life -= App->time->GetDeltaTime();
		currentParticle->colorFrame += App->time->GetDeltaTime();
	} else {
		currentParticle->life -= App->time->GetRealTimeDeltaTime();
		currentParticle->colorFrame += App->time->GetRealTimeDeltaTime();
	}
}

void ComponentParticleSystem::Init() {
	CreateParticles(maxParticles, velocity);
}

void ComponentParticleSystem::SpawnParticle() {
	Particle* currentParticle = particles.Obtain();
	if (!looping) {
		particleSpawned++;
	}
	particleSpawned++;
	if (currentParticle) {
		currentParticle->position = currentParticle->initialPosition;
		currentParticle->life = particleLife;
		if (isRandomFrame) {
			currentParticle->currentFrame = static_cast<float>(rand() % ((Xtiles * Ytiles) + 1));
		} else {
			currentParticle->currentFrame = 0;
		}
		currentParticle->colorFrame = 0;
		currentParticle->direction = CreateDirection();
		currentParticle->initialPosition = CreatePosition();
		currentParticle->position = currentParticle->initialPosition;
		//TODO: not hardcoded
		currentParticle->scale = float3(0.1f, 0.1f, 0.1f) * scale;

		if (billboardType == BillboardType::STRETCH) {
			float3x3 newRotation = float3x3::FromEulerXYZ(0.f, 0.f, pi / 2);
			currentParticle->modelStretch = currentParticle->model * newRotation;
		}
	}
}

void ComponentParticleSystem::killParticles() {
	for (Particle& currentParticle : particles) {
		particles.Release(&currentParticle);
	}
}

void ComponentParticleSystem::DrawGizmos() {
	//TODO: IMPROVE DRAWS
	if (IsActive()) {
		if (emitterType == EmitterType::CONE) {
			ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();
			dd::cone(transform->GetGlobalPosition(), transform->GetGlobalRotation() * float3::unitY * 1, dd::colors::White, coneRadiusUp, coneRadiusDown);
		}
		if (emitterType == EmitterType::SPHERE) {
			float delta = kl * kl - 4 * (kc - 10) * kq;
			float distance = Max(abs((-kl + sqrt(delta))) / (2 * kq), abs((-kl - sqrt(delta)) / (2 * kq)));
			ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();
			dd::sphere(transform->GetGlobalPosition(), dd::colors::White, 1.0f);
		}
	}
}

float4 ComponentParticleSystem::GetTintColor() const {
	ComponentButton* button = GetOwner().GetComponent<ComponentButton>();
	if (button != nullptr) {
		return button->GetTintColor();
	}
	return float4::one;
}

void ComponentParticleSystem::Draw() {
	if (isPlaying) {
		for (Particle& currentParticle : particles) {
			unsigned int program = App->programs->billboard;

			glDepthMask(GL_FALSE);
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);

			glBindBuffer(GL_ARRAY_BUFFER, App->userInterface->GetQuadVBO());
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*) (sizeof(float) * 6 * 3));
			glUseProgram(program);

			ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();

			float3x3 rotatePart = currentParticle.model.RotatePart();
			float3x3 transformRotatePart = transform->GetGlobalMatrix().RotatePart();
			Frustum* frustum = App->camera->GetActiveCamera()->GetFrustum();
			float4x4* proj = &App->camera->GetProjectionMatrix();
			float4x4* view = &App->camera->GetViewMatrix();

			float4x4 modelMatrix;
			if (billboardType == BillboardType::LOOK_AT) {
				float4x4 newModelMatrix = currentParticle.model.LookAt(rotatePart.Col(2), -frustum->Front(), rotatePart.Col(1), float3::unitY);
				modelMatrix = float4x4::FromTRS(currentParticle.position, newModelMatrix.RotatePart(), currentParticle.scale);
			} else if (billboardType == BillboardType::STRETCH) {
				float3 cameraPos = App->camera->GetActiveCamera()->GetFrustum()->Pos();
				float3 cameraDir = (cameraPos - currentParticle.initialPosition).Normalized();
				float3 upDir = Cross(currentParticle.direction, cameraDir);
				float3 newCameraDir = Cross(currentParticle.direction, upDir);

				float3x3 newRotation;
				newRotation.SetCol(0, upDir);
				newRotation.SetCol(1, currentParticle.direction);
				newRotation.SetCol(2, newCameraDir);

				modelMatrix = float4x4::FromTRS(currentParticle.position, newRotation * currentParticle.modelStretch.RotatePart(), currentParticle.scale);
			} else if (billboardType == BillboardType::HORIZONTAL) {
				float4x4 newModelMatrix = currentParticle.model.LookAt(rotatePart.Col(2), float3::unitY, rotatePart.Col(1), float3::unitY);
				modelMatrix = float4x4::FromTRS(currentParticle.position, newModelMatrix.RotatePart(), currentParticle.scale);
			} else if (billboardType == BillboardType::VERTICAL) {
				// TODO: Implement it
				modelMatrix = currentParticle.model;
			}

			glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_TRUE, modelMatrix.ptr());
			glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_TRUE, view->ptr());
			glUniformMatrix4fv(glGetUniformLocation(program, "proj"), 1, GL_TRUE, proj->ptr());

			if (!isRandomFrame) {
				if (App->time->IsGameRunning()) {
					currentParticle.currentFrame += animationSpeed * App->time->GetDeltaTime();
				} else {
					currentParticle.currentFrame += animationSpeed * App->time->GetRealTimeDeltaTime();
				}
			}

			glActiveTexture(GL_TEXTURE0);
			glUniform1i(glGetUniformLocation(program, "diffuse"), 0);
			glUniform1f(glGetUniformLocation(program, "currentFrame"), currentParticle.currentFrame);
			glUniform1f(glGetUniformLocation(program, "colorFrame"), currentParticle.colorFrame);
			glUniform4fv(glGetUniformLocation(program, "initColor"), 1, initC.ptr());
			glUniform4fv(glGetUniformLocation(program, "finalColor"), 1, finalC.ptr());
			glUniform1f(glGetUniformLocation(program, "startTransition"), startTransition);
			glUniform1f(glGetUniformLocation(program, "endTransition"), endTransition);

			glUniform1i(glGetUniformLocation(program, "Xtiles"), Xtiles);
			glUniform1i(glGetUniformLocation(program, "Ytiles"), Ytiles);

			glUniform1i(glGetUniformLocation(program, "flipX"), flipTexture[0]);
			glUniform1i(glGetUniformLocation(program, "flipY"), flipTexture[1]);

			ResourceTexture* textureResource = App->resources->GetResource<ResourceTexture>(textureID);
			if (textureResource != nullptr) {
				glBindTexture(GL_TEXTURE_2D, textureResource->glTexture);
			}
			//TODO: implement drawarrays
			//glDrawArraysInstanced(GL_TRIANGLES, 0, 4, 100);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
		}
	}
}

void ComponentParticleSystem::Play() {
	if (!isPlaying) {
		isPlaying = true;
		particleSpawned = 0;
		restDelayTime = startDelay;
	}
}

void ComponentParticleSystem::Stop() {
	particleSpawned = maxParticles;
	executer = true;
}