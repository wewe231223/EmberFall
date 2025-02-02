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

//------------------------------------------------------------------------------------------------[ ShaderManager ]------------------------------------------------------------------------------------------------

ShaderManager::ShaderManager() {
	std::ifstream metadataFile{ SHADER_METADATA_PATH };
	CrashExp(metadataFile.is_open(), "File not found");

	std::string line{};
	std::regex shaderRegex{ R"((\w+\.hlsl)\s*\{\s*((?:\w+\s+\w+\s+\w+\b\s*)+)\}\s*(?:<\s*([\w\.]+(?:\s*,\s*[\w\.]+)*)\s*>)?)" };

	while (std::getline(metadataFile, line)) {
		std::smatch match;
		if (!std::regex_match(line, match, shaderRegex)) {
			Console.Log("Invalid shader metadata format : {}", LogType::Error, line);
			continue;
		} 

		std::string	source{ match[1] };
		std::istringstream shaderInfoStream{ match[2] };
		std::vector<std::tuple<std::string, std::string, std::string>> shaderInfo{};
		std::string type{};
		std::string	model{};
		std::string entry{};
		
		while (shaderInfoStream >> type >> model >> entry) {
			shaderInfo.emplace_back(type, model, entry);
		}

		std::vector<std::filesystem::path> includes{};
		if (match[3].matched) {
			std::istringstream shaderIncludeStream{ match[3] };
			std::filesystem::path includeFile{};

			while (shaderIncludeStream >> includeFile) {
				includeFile = "Shader/Sources/" / includeFile;
				includes.emplace_back(includeFile);
			}
		}

		ShaderManager::ProcessShader( "Shader/Sources/" + source, shaderInfo, includes);
	}
}

void ShaderManager::ProcessShader(const std::filesystem::path& source, const std::vector<std::tuple<std::string, std::string, std::string>>& shaderInfo, const std::vector<std::filesystem::path>& includes) {
	static std::filesystem::path binPath{ "Shader/Binarys/" };

	if (not std::filesystem::exists(source)) {
		Console.Log("Shader source file not found : {}", LogType::Error, source.string());
		return; 
	}

	auto lastSourceWriteTime{ std::filesystem::last_write_time(source) };
	for (const auto& include : includes) {
		if (std::filesystem::exists(include) and std::filesystem::last_write_time(include) > lastSourceWriteTime) {
			lastSourceWriteTime = std::filesystem::last_write_time(include);
		}
	}

	std::filesystem::path firstBinaryPath;
	for (const auto& [type, model, entry] : shaderInfo) {
		std::string binaryFileName{ source.stem().string() + "_" + type + ".bin" };
		std::filesystem::path binaryPath{ binPath / binaryFileName };

		if (std::filesystem::exists(binaryPath)) {
			firstBinaryPath = binaryPath;
			break;
		}
	}



	if (firstBinaryPath.empty() or std::filesystem::last_write_time(firstBinaryPath) <= lastSourceWriteTime) {
		// 재 컴파일 이후 바이너리 파일에 쓰고 맵에 등록까지 완료 
		for (const auto& [type, model, entry] : shaderInfo) {
			ShaderManager::ReCompile(source, type, model, entry);
		} 
	}
	else {
		for (const auto& [type, model, entry] : shaderInfo) {
			std::string binaryFileName{ source.stem().string() + "_" + type + ".bin" };
			std::filesystem::path binaryPath{ binPath / binaryFileName };

			ComPtr<ID3D12Blob> shaderBlob{};
			ShaderManager::Load(shaderBlob, binaryPath);

			ShaderType eType{};

			if (type == "vs") {
				eType = ShaderType::VertexShader;
			}
			else if (type == "ps") {
				eType = ShaderType::PixelShader;
			}
			else if (type == "gs") {
				eType = ShaderType::GeometryShader;
			}
			else if (type == "hs") {
				eType = ShaderType::HullShader;
			}
			else if (type == "ds") {
				eType = ShaderType::DomainShader;
			}
			else {
				Crash(("Invalid Shaer Type : " + type).c_str());
			}

			mShaderBlobs[source.stem().string()][eType] = shaderBlob;
		}
	}

}

void ShaderManager::ReCompile(const std::filesystem::path& source, const std::string& type, const std::string& model, const std::string& entry) {
	Console.Log("Compiling shader : {}", LogType::Info, source.string());

	ComPtr<ID3D12Blob> shaderBlob{};
	ComPtr<ID3D12Blob> errorBlob{};
	
	auto hr = ::D3DCompileFromFile(source.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry.c_str(), model.c_str(), 0, 0, &shaderBlob, &errorBlob);

	std::string binaryFileName{ source.stem().string() + "_" + type + ".bin" };
	std::filesystem::path binaryPath{ "Shader/Binarys/" + binaryFileName };

	if (SUCCEEDED(hr)) {
		std::ofstream binaryFile{ binaryPath, std::ios::binary };
		CrashExp(binaryFile.is_open(), "Failed to create binary file");

		binaryFile.write(static_cast<char*>(shaderBlob->GetBufferPointer()), shaderBlob->GetBufferSize());
	}
	else {
		Console.Log("Failed to compile shader : {}\nLoad previous version!", LogType::Error, reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
		CrashExp(std::filesystem::exists(binaryPath), "Binary file not found");
		
		if (shaderBlob) {
			shaderBlob->Release();
		}

		ShaderManager::Load(shaderBlob, binaryPath);
	}
	
	ShaderType eType{};

	if (type == "vs") {
		eType = ShaderType::VertexShader;
	}
	else if (type == "ps") {
		eType = ShaderType::PixelShader;
	}
	else if (type == "gs") {
		eType = ShaderType::GeometryShader;
	}
	else if (type == "hs") {
		eType = ShaderType::HullShader;
	}
	else if (type == "ds") {
		eType = ShaderType::DomainShader;
	}
	else {
		Crash(("Invalid Shaer Type : " + type).c_str());
	}

	mShaderBlobs[source.stem().string()][eType] = shaderBlob;
}

void ShaderManager::Load(ComPtr<ID3D12Blob>& blob, const std::filesystem::path& path) {
	auto size = std::filesystem::file_size(path);

	::D3DCreateBlob(size, blob.GetAddressOf());

	std::ifstream file{ path, std::ios::binary };
	file.read(static_cast<char*>(blob->GetBufferPointer()), size);
}

ShaderManager gShaderManager{};

//------------------------------------------------------------------------------------------------[ ShaderManager ]------------------------------------------------------------------------------------------------
