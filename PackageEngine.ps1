${EngineName} = "Tesseract"
${Version} = "0.3.1"

${ReleaseDir} = "Build/Release"
${GameReleaseDir} = "Build/Game"
${ProjectDir} = "Project"
${GameDir} = "Game"
${EngineSourceDir} = "Engine"
${AssetsDir} = "Assets"
${ShadersDir} = "Shaders"
${LibraryDir} = "Library"
${GameProjectDir} = "Penteract"
${LicensesDir} = "Licenses"

${PackageDir} = "Build/Package"
${OutDir} = "Build"

Remove-Item -Path "${PackageDir}" -ErrorAction "Ignore" -Recurse
New-Item -Path "${PackageDir}" -ItemType "Directory" -Force
New-Item -Path "${PackageDir}/${EngineName}" -ItemType "Directory" -Force
New-Item -Path "${PackageDir}/${EngineName}/${EngineSourceDir}" -ItemType "Directory" -Force
Copy-Item -Path "${LicensesDir}", "LICENSE", "README.md" -Destination "${PackageDir}" -Force -Recurse
Copy-Item -Path "${GameDir}/*", "${ReleaseDir}/${EngineName}.exe", "${ReleaseDir}/${EngineName}.lib" -Exclude "*.ini", "${AssetsDir}", "${LibraryDir}", "${GameProjectDir}" -Destination "${PackageDir}/${EngineName}" -Force -Recurse
Copy-Item -Path "${GameDir}/${AssetsDir}/${ShadersDir}" -Destination "${PackageDir}/${EngineName}/${AssetsDir}/${ShadersDir}" -Force -Recurse
Copy-Item -Path "${GameReleaseDir}/${EngineName}.exe" -Destination "${PackageDir}/${EngineName}/${EngineSourceDir}" -Force -Recurse
Copy-Item -Path "${ProjectDir}/*" -Exclude "${EngineName}.user", "${EngineName}.filters", "${EngineName}.vcxproj", "${EngineName}.vcxproj.*", ".clang-format", "Debugging.props" -Destination "${PackageDir}/${EngineName}/${EngineSourceDir}" -Force -Recurse
Compress-Archive -Path "${PackageDir}/*" -DestinationPath "${OutDir}/${EngineName}_${Version}.zip" -Force