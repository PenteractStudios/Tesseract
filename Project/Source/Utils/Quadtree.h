#pragma once

#include "Globals.h"
#include "Utils/Pool.h"

#include "Math/myassert.h"
#include "Geometry/AABB2D.h"
#include <vector>
#include <list>

template<typename T>
class Quadtree {
public:
	// The elements in a node are linked
	class Element {
	public:
		T* object = nullptr;
		AABB2D aabb = {{0, 0}, {0, 0}};
		Element* next = nullptr;
	};

	// Nodes are as small as possible (8 bytes) to reduce memory usage and cache efficiency
	class QuadNode;
	class Node {
	public:
		void Add(Quadtree& tree, T* object, const AABB2D& objectAABB, unsigned depth, const AABB2D& nodeAABB, bool optimizing) {
			if (IsBranch()) {
				// Branch
				childNodes->Add(tree, object, objectAABB, depth + 1, nodeAABB, optimizing);
			} else if (depth == tree.maxDepth || (unsigned) elementCount < tree.maxNodeElements) {
				// Leaf that can't split or leaf with space
				if (optimizing) {
					Element* newFirstElement = tree.elements.Obtain();
					newFirstElement->object = object;
					newFirstElement->aabb = objectAABB;
					newFirstElement->next = firstElement;
					firstElement = newFirstElement;
				} else {
					Element element;
					element.object = object;
					element.aabb = objectAABB;
					size_t index = tempElementList->size();
					tempElementList->emplace_back(element);
					tree.numAddedElements += 1;
				}
				elementCount += 1;
			} else {
				// Leaf with no space that can split
				Split(tree, depth, nodeAABB, optimizing);
				childNodes->Add(tree, object, objectAABB, depth + 1, nodeAABB, optimizing);
			}
		}

		void Remove(Quadtree& tree, T* object) {
			if (IsBranch()) {
				childNodes->Remove(tree, object);
			} else {
				Element** elementPtr = &firstElement;
				Element* element = firstElement;
				while (element != nullptr) {
					if (element->object == object) {
						*elementPtr = element->next;
						tree.elements.Release(element);
						elementCount -= 1;
					}
					elementPtr = &element->next;
					element = element->next;
				}
			}
		}

		void Split(Quadtree& tree, unsigned depth, const AABB2D& nodeAABB, bool optimizing) {
			if (optimizing) {
				// Get first element before anything changes
				Element* element = firstElement;

				// Transform leaf into branch
				elementCount = -1;
				childNodes = tree.quadNodes.Obtain();
				childNodes->Initialize();

				// Remove all elements and reinsert them
				while (element != nullptr) {
					T* object = element->object;
					AABB2D objectAABB = element->aabb;
					Element* nextElement = element->next;
					tree.elements.Release(element);

					childNodes->Add(tree, object, objectAABB, depth + 1, nodeAABB, optimizing);

					element = nextElement;
				}
			} else {
				// Get element vector before anything changes
				std::list<Element>* tempElements = tempElementList;

				// Transform leaf into branch
				elementCount = -1;
				tree.auxQuadNodes.emplace_back(QuadNode());
				childNodes = &tree.auxQuadNodes.back();
				childNodes->InitializeAndCreateElementLists();

				// Remove all elements and reinsert them
				for (Element& tempElement : *tempElements) {
					T* object = tempElement.object;
					AABB2D objectAABB = tempElement.aabb;
					tree.numAddedElements -= 1;

					childNodes->Add(tree, object, objectAABB, depth + 1, nodeAABB, optimizing);
				}

				RELEASE(tempElements);
			}
		}

		bool IsLeaf() const {
			return elementCount >= 0;
		}

		bool IsBranch() const {
			return elementCount < 0;
		}

	public:
		int elementCount = 0; // Leaf: number of elements. Branch: -1.
		union {
			Element* firstElement = nullptr; // Leaf only: first element.
			QuadNode* childNodes;			 // Branch only: child nodes index.
			std::list<Element>* tempElementList;
		};
	};

	// Nodes are in groups of 4 so that only 1 pointer is needed
	struct QuadNode {
	public:
		void Initialize() {
			for (Node& node : nodes) {
				node.elementCount = 0;
				node.firstElement = nullptr;
			}
		}

		void InitializeAndCreateElementLists() {
			for (Node& node : nodes) {
				node.elementCount = 0;
				node.tempElementList = new std::list<Element>();
			}
		}

		void ReleaseElementLists() {
			for (Node& node : nodes) {
				if (node.IsLeaf()) {
					RELEASE(node.tempElementList);
				}
			}
		}

		void Add(Quadtree& tree, T* object, const AABB2D& objectAABB, unsigned depth, const AABB2D& nodeAABB, bool optimizing) {
			vec2d center = nodeAABB.minPoint + (nodeAABB.maxPoint - nodeAABB.minPoint) * 0.5f;

			AABB2D topLeftAABB = {{nodeAABB.minPoint.x, center.y}, {center.x, nodeAABB.maxPoint.y}};
			if (objectAABB.Intersects(topLeftAABB)) {
				nodes[0].Add(tree, object, objectAABB, depth, topLeftAABB, optimizing);
			}

			AABB2D topRightAABB = {{center.x, center.y}, {nodeAABB.maxPoint.x, nodeAABB.maxPoint.y}};
			if (objectAABB.Intersects(topRightAABB)) {
				nodes[1].Add(tree, object, objectAABB, depth, topRightAABB, optimizing);
			}

			AABB2D bottomLeftAABB = {{nodeAABB.minPoint.x, nodeAABB.minPoint.y}, {center.x, center.y}};
			if (objectAABB.Intersects(bottomLeftAABB)) {
				nodes[2].Add(tree, object, objectAABB, depth, bottomLeftAABB, optimizing);
			}

			AABB2D bottomRightAABB = {{center.x, nodeAABB.minPoint.y}, {nodeAABB.maxPoint.x, center.y}};
			if (objectAABB.Intersects(bottomRightAABB)) {
				nodes[3].Add(tree, object, objectAABB, depth, bottomRightAABB, optimizing);
			}
		}

		void Remove(Quadtree& tree, T* object) {
			for (Node& node : nodes) {
				node.Remove(tree, object);
			}
		}

	public:
		Node nodes[4];
	};

public:
	void Initialize(AABB2D quadtreeBounds, unsigned quadtreeMaxDepth, unsigned maxElementsPerNode) {
		assert(quadtreeMaxDepth > 0);

		Clear();

		bounds = quadtreeBounds;
		maxDepth = quadtreeMaxDepth;
		maxNodeElements = maxElementsPerNode;

		auxRoot.tempElementList = new std::list<Element>();
	}

	void Add(T* object, const AABB2D& objectAABB) {
		assert(!operative); // Tried to add an object to a locked quadtree

		auxRoot.Add(*this, object, objectAABB, 1, bounds, false);
		addedObjects.emplace_back(std::pair<T*, AABB2D>(object, objectAABB));
	}

	void Remove(T* object) {
		assert(operative); // Tried to remove an object from an unlocked quadtree

		root.Remove(*this, object);
	}

	void Optimize() {
		quadNodes.Allocate(auxQuadNodes.size());
		elements.Allocate(numAddedElements);

		for (std::pair<T*, AABB2D> pair : addedObjects) {
			AddToPools(pair.first, pair.second);
		}

		for (QuadNode& quadNode : auxQuadNodes) {
			quadNode.ReleaseElementLists();
		}

		if (auxRoot.IsBranch()) {
			auxRoot.elementCount = 0;
			auxRoot.tempElementList = nullptr;
		} else {
			auxRoot.elementCount = 0;
			RELEASE(auxRoot.tempElementList);
		}
		numAddedElements = 0;
		auxQuadNodes.clear();
		addedObjects.clear();

		operative = true;
	}

	bool IsOperative() {
		return operative;
	}

	void Clear() {
		bounds.minPoint.Set(0, 0);
		bounds.maxPoint.Set(0, 0);
		maxDepth = 0;
		maxNodeElements = 0;

		root.elementCount = 0;
		root.firstElement = nullptr;
		quadNodes.Deallocate();
		elements.Deallocate();

		operative = false;

		for (QuadNode& quadNode : auxQuadNodes) {
			quadNode.ReleaseElementLists();
		}

		if (auxRoot.IsBranch()) {
			auxRoot.elementCount = 0;
			auxRoot.tempElementList = nullptr;
		} else {
			auxRoot.elementCount = 0;
			RELEASE(auxRoot.tempElementList);
		}
		numAddedElements = 0;
		auxQuadNodes.clear();
		addedObjects.clear();
	}

public:
	AABB2D bounds = {{0, 0}, {0, 0}}; // Bounds of the quadtree. All elements should be contained inside this.
	unsigned maxDepth = 0;			  // Max depth of the tree. Useful to avoid infinite divisions. This should be >= 1.
	unsigned maxNodeElements = 0;	  // Max number of elements before a node is divided.

	Node root;
	Pool<QuadNode> quadNodes;
	Pool<Element> elements;

protected:
	void AddToPools(T* object, const AABB2D& objectAABB) {
		root.Add(*this, object, objectAABB, 1, bounds, true);
	}

private:
	bool operative = false;

	Node auxRoot;
	unsigned numAddedElements = 0;
	std::list<QuadNode> auxQuadNodes;
	std::list<std::pair<T*, AABB2D>> addedObjects;
};
