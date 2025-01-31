#include "pch.h"
#include "Shader.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <windows.h>
#include <d3dcompiler.h>
#include <regex>
#include "../Utility/Crash.h"
#include "../EditorInterface/Console/Console.h"

ShaderManager::ShaderManager() {
	std::ifstream metadataFile{ SHADER_METADATA_PATH };
	CrashExp(metadataFile.is_open(), "File not found");

	std::string line{};
	std::regex shaderRegex(R"((\w+\.hlsl)\s*\{\s*((?:\w+\s+\w+\s+\w+\b\s*)+)\}\s*(?:<\s*([\w\.]+(?:\s*,\s*[\w\.]+)*)\s*>)?)");

	while (std::getline(metadataFile, line)) {
		std::smatch match;
		if (!std::regex_match(line, match, shaderRegex)) {
			Console.Log("Invalid shader metadata format : {}", LogType::Error, line);
			continue;
		} 

		std::string	source{ match[1] };
		std::istringstream shaderInfoStream{ match[2] };
		std::vector<std::tuple<ShaderType, std::string, std::string>> shaderInfo{};
		std::string type{};
		std::string	model{};
		std::string entry{};

		while (shaderInfoStream >> type >> model >> entry) {
			ShaderType eType{};

			// fuck 
            if (type == "vs") {
                eType = ShaderType::VertexShader;
            } else if (type == "ps") {
                eType = ShaderType::PixelShader;
            } else if (type == "gs") {
                eType = ShaderType::GeometryShader;
            } else if (type == "hs") {
                eType = ShaderType::HullShader;
            } else if (type == "ds") {
                eType = ShaderType::DomainShader;
            } else {
                Console.Log("Invalid shader type : {}", LogType::Error, type);
            }

			shaderInfo.emplace_back(eType, model, entry);
		}

		std::vector<std::string> includes{};
		if (match[3].matched) {
			std::istringstream shaderIncludeStream{ match[3] };
			std::string includeFile{};

			while (shaderIncludeStream >> includeFile) {
				includes.emplace_back("Shaders/Sources/" + includeFile);
			}
		}



	}

}
