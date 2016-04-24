#include "SPIRV/GlslangToSpv.h"
#include <vulkan/vulkan.h>

#include "Kaleido3D.h"
#include <Core/LogUtil.h>

#include "GLSLCompiler.h"
#include "Spir2Cross.hpp"
#include "spir2glsl.hpp"

namespace k3d {

	IShaderCompilerOutput* GLSLCompiler::Compile(ShaderCompilerOption const& option,  const char * source)
	{
		return glslToSpv(option.ShaderType, source);
	}

	void GLSLCompiler::Reflect(const char* byteCode, uint32 length)
	{
			//auto backCompiler = std::unique_ptr<spir2cross::CompilerGLSL>(new spir2cross::CompilerGLSL(shader.spirvBinary));
			//if (!backCompiler) {
			//	std::string msg = "Couldn't back compile SPIR-V for shader stage " + toStringVkShaderStageFlagBits(shader.stage);
			//	throw std::runtime_error(msg);
			//}

			//// Extract shader attributes
			//if (VK_SHADER_STAGE_VERTEX_BIT == shader.stage) {
			//	extractAttributeData(backCompiler, &mAttributes);
			//}

			//// Extract uniform data
			//if (!format.userDefinedUniformLayout()) {
			//	extractUniformData(shader.stage, backCompiler, &mUniformLayout);
			//}
	}

	using namespace ::glslang;
	using namespace shaderbinding;

	EDataType spirTypeToGlslAttributeDataType(const spir2cross::SPIRType& spirType)
	{
		EDataType result = EDataType::EUnknown;
		switch (spirType.basetype)
		{
		case spir2cross::SPIRType::Bool: 
		{
			switch (spirType.vecsize) 
			{
			case 1: result = EDataType::EBool; break;
			case 2: result = EDataType::EBool2; break;
			case 3: result = EDataType::EBool3; break;
			case 4: result = EDataType::EBool4; break;
			}
		}
		break;

		case spir2cross::SPIRType::Int: 
		{
			switch (spirType.vecsize) 
			{
			case 1: result = EDataType::EInt; break;
			case 2: result = EDataType::EInt2; break;
			case 3: result = EDataType::EInt3; break;
			case 4: result = EDataType::EInt4; break;
			}
		}
		break;

		case spir2cross::SPIRType::UInt:
		{
			switch (spirType.vecsize)
			{
			case 1: result = EDataType::EUInt; break;
			case 2: result = EDataType::EUInt2; break;
			case 3: result = EDataType::EUInt3; break;
			case 4: result = EDataType::EUInt4; break;
			}
		}
		break;

		case spir2cross::SPIRType::Float: 
		{
			switch (spirType.vecsize) 
			{
			case 1: result = EDataType::EFloat; break;
			case 2: result = EDataType::EFloat2; break;
			case 3: result = EDataType::EFloat3; break;
			case 4: result = EDataType::EFloat4; break;
			}
		}
		break;

		}
		return result;
	}

	ESemantic attributeNameToSemantic(std::string const& attribName)
	{
		if (attribName == "POSITION")
		{
			return ESemantic::EPOSITION;
		}
		else if (attribName == "COLOR")
		{
			return ESemantic::ECOLOR;
		}
		else if (attribName == "NORMAL")
		{
			return ESemantic::ENORMAL;
		}

		return ESemantic::ENumSemanics;
	}

	EDataType spirTypeToGlslUniformDataType(const spir2cross::SPIRType& spirType)
	{
		EDataType result = EDataType::EUnknown;
		switch (spirType.basetype) 
		{
		case spir2cross::SPIRType::Bool: 
		{
			switch (spirType.vecsize) 
			{
			case 1: result = EDataType::EBool; break;
			case 2: result = EDataType::EBool2; break;
			case 3: result = EDataType::EBool3; break;
			case 4: result = EDataType::EBool4; break;
			}
		}
		break;

		case spir2cross::SPIRType::Int: 
		{
			switch (spirType.vecsize)
			{
			case 1: result = EDataType::EInt; break;
			case 2: result = EDataType::EInt2; break;
			case 3: result = EDataType::EInt3; break;
			case 4: result = EDataType::EInt4; break;
			}
		}
		break;

		case spir2cross::SPIRType::UInt: 
		{
			switch (spirType.vecsize) 
			{
			case 1: result = EDataType::EUInt; break;
			case 2: result = EDataType::EUInt2; break;
			case 3: result = EDataType::EUInt3; break;
			case 4: result = EDataType::EUInt4; break;
			}
		}
		break;

		case spir2cross::SPIRType::Float: 
		{
			switch (spirType.columns) 
			{
			case 1: 
			{
				switch (spirType.vecsize) 
				{
				case 1: result = EDataType::EFloat; break;
				case 2: result = EDataType::EFloat2; break;
				case 3: result = EDataType::EFloat3; break;
				case 4: result = EDataType::EFloat4; break;
				}
			}
			break;

			case 2: 
			{
				switch (spirType.vecsize)
				{
				case 2: result = EDataType::EMat2; break;
				case 3: result = EDataType::EMat2x3; break;
				case 4: result = EDataType::EMat2x4; break;
				}
			}
			break;

			case 3: 
			{
				switch (spirType.vecsize)
				{
				case 2: result = EDataType::EMat3x2; break;
				case 3: result = EDataType::EMat3; break;
				case 4: result = EDataType::EMat3x4; break;
				}
			}
			break;

			case 4: 
			{
				switch (spirType.vecsize) 
				{
				case 2: result = EDataType::EMat4x2; break;
				case 3: result = EDataType::EMat4x3; break;
				case 4: result = EDataType::EMat4; break;
				}
			}
			break;
			}
		}
		break;
		}
		return result;
	}

	void extractAttributeData(const std::unique_ptr<spir2cross::CompilerGLSL>& backCompiler, std::vector<shaderbinding::Attribute> *outShaderAttributes)
	{
		if (!backCompiler) 
		{
			return;
		}

		for (auto& res : backCompiler->get_shader_resources().stage_inputs) 
		{
			spir2cross::SPIRType	spirType = backCompiler->get_type(res.type_id);
			std::string				attrName = res.name;
			uint32					attrLocation = backCompiler->get_decoration(res.id, spv::DecorationLocation);
			uint32					attrBinding = 0;

			EDataType				attrDataType = spirTypeToGlslAttributeDataType(spirType);
			ESemantic				attrSemantic = attributeNameToSemantic(res.name);

			if (EDataType::EUnknown == attrDataType) {
				Log::Out(LogLevel::Error, "GLSLCompiler", "Unable to determine data type for vertex shader attr %s",attrName.c_str());
				continue;
			}

			auto it = std::find_if(
				std::begin(*outShaderAttributes), std::end(*outShaderAttributes),
				[attrName](const Attribute& elem) -> bool 
				{
					return elem.VarName == attrName;
				}
			);

			if (std::end(*outShaderAttributes) != it) 
			{
				Attribute& attr = *it;
				attr.VarLocation = attrLocation;
				attr.VarBindingPoint = attrBinding;
				attr.VarType = attrDataType;
				attr.VarSemantic = attrSemantic;
			}
			else 
			{
				Attribute attr = { attrName, attrSemantic, attrDataType, static_cast<int32>(attrLocation), attrBinding };
				outShaderAttributes->push_back(attr);
			}
		}

		std::sort(
			std::begin(*outShaderAttributes), std::end(*outShaderAttributes),
			[](const Attribute& a, const Attribute& b) -> bool
			{
				return a.VarLocation < b.VarLocation;
			}
		);
	}

	//void extractBlock(
	//	VkShaderStageFlagBits shaderStage, 
	//	const spir2cross::Resource& res, 
	//	const std::unique_ptr<spir2cross::CompilerGLSL>& backCompiler,
	//	Uniform *outUniformLayout)
	//{
	//	const uint64_t kBlockMask = (1ULL << spv::DecorationBlock) | (1ULL << spv::DecorationBufferBlock);

	//	auto&    typeInfo = backCompiler->get_type(res.type_id);
	//	bool     isPushConstant = (spv::StorageClassPushConstant == backCompiler->get_storage_class(res.id));
	//	bool     isBlock = (0 != (backCompiler->get_decoration_mask(typeInfo.self) & kBlockMask));
	//	uint32 typeId = ((!isPushConstant && isBlock) ? res.type_id : res.id);

	//	auto                  bindingType = isPushConstant ? vk::UniformLayout::Binding::Type::PUSH_CONSTANTS : vk::UniformLayout::Binding::Type::BLOCK;
	//	std::string           bindingName = backCompiler->get_name(res.id);
	//	uint32              bindingNumber = backCompiler->get_decoration(res.id, spv::DecorationBinding);
	//	uint32              bindingSet = backCompiler->get_decoration(res.id, spv::DecorationDescriptorSet);
	//	VkShaderStageFlagBits bindingStage = shaderStage;

	//	outUniformLayout->addBinding(bindingType, bindingName, bindingNumber, bindingStage, bindingSet);
	//	outUniformLayout->addSet(bindingSet, CHANGES_DONTCARE);

	//	for (uint32 index = 0; index < typeInfo.member_types.size(); ++index) {
	//		std::string         memberName = backCompiler->get_member_name(res.type_id, index);
	//		uint32            memberId = typeInfo.member_types[index];
	//		auto&               memberTypeInfo = backCompiler->get_type(memberId);
	//		uint32            memberOffset = backCompiler->get_member_decoration(res.type_id, index, spv::DecorationOffset);
	//		uint32            memberArraySize = memberTypeInfo.array.empty() ? 1 : memberTypeInfo.array[0];
	//		EDataType			memberDataType = spirTypeToGlslUniformDataType(memberTypeInfo);

	//		std::string uniformName = bindingName + "." + memberName;
	//		outUniformLayout->addUniform(uniformName, memberDataType, memberOffset, memberArraySize);
	//	}

	//	// Block size
	//	size_t blockSize = backCompiler->get_declared_struct_size(typeInfo);

	//	// Round up to next multiple of 16			
	//	size_t paddedSize = (blockSize + 15) & (~15);
	//	if ((blockSize != paddedSize) && (!isPushConstant)) {
	//		Log::Out(LogLevel::Info, "GLSLCompiler", "Padded uniform block " << bindingName << " from " << blockSize << " bytes to " << paddedSize << " bytes");
	//	}

	//	// Use block size for push constants and padded size for UBOs
	//	outUniformLayout->setBlockSizeBytes(bindingName, isPushConstant ? blockSize : paddedSize);
	//	outUniformLayout->sortByOffset();
	//}

	//void extractUniformData(
	//	VkShaderStageFlagBits shaderStage, 
	//	const std::unique_ptr<spir2cross::CompilerGLSL>& backCompiler, 
	//	Uniform *outUniformLayout)
	//{
	//	if (!backCompiler) {
	//		return;
	//	}

	//	// Extract uniform blocks from all shader stages
	//	for (auto& res : backCompiler->get_shader_resources().uniform_buffers) {
	//		extractBlock(shaderStage, res, backCompiler, outUniformLayout);
	//	}

	//	// Extract samplers from all shader stages
	//	for (auto& res : backCompiler->get_shader_resources().sampled_images) {
	//		std::string           bindingName = backCompiler->get_name(res.id);
	//		uint32              bindingNumber = backCompiler->get_decoration(res.id, spv::DecorationBinding);
	//		uint32              bindingSet = backCompiler->get_decoration(res.id, spv::DecorationDescriptorSet);
	//		VkShaderStageFlagBits bindingStage = shaderStage;

	//		outUniformLayout->addBinding(vk::UniformLayout::Binding::Type::SAMPLER, bindingName, bindingNumber, bindingStage, bindingSet);
	//		outUniformLayout->addSet(bindingSet, CHANGES_DONTCARE);
	//	}

	//	// Extract storage images from all shader stages - but probably only just compute
	//	for (auto& res : backCompiler->get_shader_resources().storage_images) {
	//		std::string				bindingName = backCompiler->get_name(res.id);
	//		uint32					bindingNumber = backCompiler->get_decoration(res.id, spv::DecorationBinding);
	//		uint32					bindingSet = backCompiler->get_decoration(res.id, spv::DecorationDescriptorSet);
	//		VkShaderStageFlagBits	bindingStage = shaderStage;

	//		outUniformLayout->addBinding(vk::UniformLayout::Binding::Type::STORAGE_IMAGE, bindingName, bindingNumber, bindingStage, bindingSet);
	//		outUniformLayout->addSet(bindingSet, CHANGES_DONTCARE);
	//	}

	//	// Extract storage buffers from all shader stages - but probably only just compute
	//	for (auto& res : backCompiler->get_shader_resources().storage_buffers) {
	//		std::string				bindingName = backCompiler->get_name(res.id);
	//		uint32					bindingNumber = backCompiler->get_decoration(res.id, spv::DecorationBinding);
	//		uint32					bindingSet = backCompiler->get_decoration(res.id, spv::DecorationDescriptorSet);
	//		VkShaderStageFlagBits	bindingStage = shaderStage;

	//		outUniformLayout->addBinding(vk::UniformLayout::Binding::Type::STORAGE_BUFFER, bindingName, bindingNumber, bindingStage, bindingSet);
	//		outUniformLayout->addSet(bindingSet, CHANGES_DONTCARE);
	//	}

	//	// Extract push constants from all shader stages
	//	for (auto& res : backCompiler->get_shader_resources().push_constant_buffers) {
	//		extractBlock(shaderStage, res, backCompiler, outUniformLayout);
	//	}
	//}

	void initResources(TBuiltInResource &resources)
	{
		resources.maxLights = 32;
		resources.maxClipPlanes = 6;
		resources.maxTextureUnits = 32;
		resources.maxTextureCoords = 32;
		resources.maxVertexAttribs = 64;
		resources.maxVertexUniformComponents = 4096;
		resources.maxVaryingFloats = 64;
		resources.maxVertexTextureImageUnits = 32;
		resources.maxCombinedTextureImageUnits = 80;
		resources.maxTextureImageUnits = 32;
		resources.maxFragmentUniformComponents = 4096;
		resources.maxDrawBuffers = 32;
		resources.maxVertexUniformVectors = 128;
		resources.maxVaryingVectors = 8;
		resources.maxFragmentUniformVectors = 16;
		resources.maxVertexOutputVectors = 16;
		resources.maxFragmentInputVectors = 15;
		resources.minProgramTexelOffset = -8;
		resources.maxProgramTexelOffset = 7;
		resources.maxClipDistances = 8;
		resources.maxComputeWorkGroupCountX = 65535;
		resources.maxComputeWorkGroupCountY = 65535;
		resources.maxComputeWorkGroupCountZ = 65535;
		resources.maxComputeWorkGroupSizeX = 1024;
		resources.maxComputeWorkGroupSizeY = 1024;
		resources.maxComputeWorkGroupSizeZ = 64;
		resources.maxComputeUniformComponents = 1024;
		resources.maxComputeTextureImageUnits = 16;
		resources.maxComputeImageUniforms = 8;
		resources.maxComputeAtomicCounters = 8;
		resources.maxComputeAtomicCounterBuffers = 1;
		resources.maxVaryingComponents = 60;
		resources.maxVertexOutputComponents = 64;
		resources.maxGeometryInputComponents = 64;
		resources.maxGeometryOutputComponents = 128;
		resources.maxFragmentInputComponents = 128;
		resources.maxImageUnits = 8;
		resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
		resources.maxCombinedShaderOutputResources = 8;
		resources.maxImageSamples = 0;
		resources.maxVertexImageUniforms = 0;
		resources.maxTessControlImageUniforms = 0;
		resources.maxTessEvaluationImageUniforms = 0;
		resources.maxGeometryImageUniforms = 0;
		resources.maxFragmentImageUniforms = 8;
		resources.maxCombinedImageUniforms = 8;
		resources.maxGeometryTextureImageUnits = 16;
		resources.maxGeometryOutputVertices = 256;
		resources.maxGeometryTotalOutputComponents = 1024;
		resources.maxGeometryUniformComponents = 1024;
		resources.maxGeometryVaryingComponents = 64;
		resources.maxTessControlInputComponents = 128;
		resources.maxTessControlOutputComponents = 128;
		resources.maxTessControlTextureImageUnits = 16;
		resources.maxTessControlUniformComponents = 1024;
		resources.maxTessControlTotalOutputComponents = 4096;
		resources.maxTessEvaluationInputComponents = 128;
		resources.maxTessEvaluationOutputComponents = 128;
		resources.maxTessEvaluationTextureImageUnits = 16;
		resources.maxTessEvaluationUniformComponents = 1024;
		resources.maxTessPatchComponents = 120;
		resources.maxPatchVertices = 32;
		resources.maxTessGenLevel = 64;
		resources.maxViewports = 16;
		resources.maxVertexAtomicCounters = 0;
		resources.maxTessControlAtomicCounters = 0;
		resources.maxTessEvaluationAtomicCounters = 0;
		resources.maxGeometryAtomicCounters = 0;
		resources.maxFragmentAtomicCounters = 8;
		resources.maxCombinedAtomicCounters = 8;
		resources.maxAtomicCounterBindings = 1;
		resources.maxVertexAtomicCounterBuffers = 0;
		resources.maxTessControlAtomicCounterBuffers = 0;
		resources.maxTessEvaluationAtomicCounterBuffers = 0;
		resources.maxGeometryAtomicCounterBuffers = 0;
		resources.maxFragmentAtomicCounterBuffers = 1;
		resources.maxCombinedAtomicCounterBuffers = 1;
		resources.maxAtomicCounterBufferSize = 16384;
		resources.maxTransformFeedbackBuffers = 4;
		resources.maxTransformFeedbackInterleavedComponents = 64;
		resources.maxCullDistances = 8;
		resources.maxCombinedClipAndCullDistances = 8;
		resources.maxSamples = 4;
		resources.limits.nonInductiveForLoops = 1;
		resources.limits.whileLoops = 1;
		resources.limits.doWhileLoops = 1;
		resources.limits.generalUniformIndexing = 1;
		resources.limits.generalAttributeMatrixVectorIndexing = 1;
		resources.limits.generalVaryingIndexing = 1;
		resources.limits.generalSamplerIndexing = 1;
		resources.limits.generalVariableIndexing = 1;
		resources.limits.generalConstantMatrixVectorIndexing = 1;
	}

	EShLanguage findLanguage(const rhi::EShaderType shader_type)
	{
		switch (shader_type) {
		case rhi::ES_Vertex:
			return EShLangVertex;

		case rhi::ES_Hull:
			return EShLangTessControl;

		case rhi::ES_Domain:
			return EShLangTessEvaluation;

		case rhi::ES_Geometry:
			return EShLangGeometry;

		case rhi::ES_Fragment:
			return EShLangFragment;

		case rhi::ES_Compute:
			return EShLangCompute;

		default:
			return EShLangVertex;
		}
	}

	GLSLOutput* glslToSpv(
		const rhi::EShaderType shader_type,
		const char *pshader)
	{
		glslang::TProgram& program = *new glslang::TProgram;
		const char *shaderStrings[1];
		TBuiltInResource Resources;
		initResources(Resources);

		// Enable SPIR-V and Vulkan rules when parsing GLSL
		EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

		EShLanguage stage = findLanguage(shader_type);
		glslang::TShader* shader = new glslang::TShader(stage);

		shaderStrings[0] = pshader;
		shader->setStrings(shaderStrings, 1);

		GLSLOutput * output = nullptr;
		if (!shader->parse(&Resources, 100, false, messages)) {
			puts(shader->getInfoLog());
			puts(shader->getInfoDebugLog());
			output = new GLSLOutput();
			output->m_Result = EShaderCompileResult::Fail;
			output->m_ErrorMsg = program.getInfoLog();
			return output;
		}
		program.addShader(shader);
		if (!program.link(messages)) {
			puts(program.getInfoLog());
			puts(program.getInfoDebugLog());
			output = new GLSLOutput();
			output->m_Result = EShaderCompileResult::Fail;
			output->m_ErrorMsg = program.getInfoLog();
			return output;
		}
		std::vector<unsigned int> spirv;
		glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);
		rhi::ShaderByteCode bc(spirv.data(), spirv.size());
		output = new GLSLOutput(std::move(bc));
		output->m_Result = EShaderCompileResult::Success;
		return output;
	}


	static void sInitializeGlSlang()
	{
		static bool sGlSlangIntialized = false;
		if (!sGlSlangIntialized) {
			glslang::InitializeProcess();
			//sGlSlangIntialized = true;
		}
	}

	static void sFinializeGlSlang()
	{
		static bool sGlSlangFinalized = false;
		if (!sGlSlangFinalized) {
			glslang::FinalizeProcess();
			//sGlSlangFinalized = true;
		}
	}

	GLSLCompiler::GLSLCompiler()
	{
		sInitializeGlSlang();
	}

	GLSLCompiler::~GLSLCompiler()
	{
		sFinializeGlSlang();
	}
}