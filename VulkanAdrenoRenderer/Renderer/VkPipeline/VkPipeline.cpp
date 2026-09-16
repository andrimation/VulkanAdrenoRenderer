#include "VkPipeline.h"

#include "../VkContext/VkContext.h"
#include "../GLFWWindow/WindowGLFW.h"
#include "../VkSwapChain/VkSwapChain.h"

#include <iostream>
#include <fstream> // żeby odczytywać pliki

void Vk_Pipeline::CreateGraphicsPipeline(Vk_Context* InContext, WindowGLFW* InWindow, Vk_SwapChain* InSwapChain)
{
	std::vector<uint32_t> shaderCode = ReadFile("slang.spv");

	// Sprawdzenie że alignment będzie poprawny - ale zrobić żeby przechowywać jednak bity jako uint32_t
	//assert(shaderCode.size() % sizeof(uint32_t) == 0);

	vk::raii::ShaderModule shaderModule = CreateShaderModule(shaderCode,InContext);

	std::cout << "Byte:  " << sizeof(char) << "   uint32_t:  " << sizeof(uint32_t) << "\n";

	// Naszego shader module przypisujemy później do konkretnego pipeline stage - i podajemy nazwę głównej funkcji, którą ten pipeline stage
	// wywołuje

	vk::PipelineShaderStageCreateInfo vertexShaderCreationInfo
	{
		.stage = vk::ShaderStageFlagBits::eVertex,
		.module = shaderModule,
		.pName = "vertMain"  // <- i to jest nazwa funkcji "wejściowej" w shaderze dla tego stepu pipeline
	};

	vk::PipelineShaderStageCreateInfo fragmentShaderCreationInfo
	{
		.stage = vk::ShaderStageFlagBits::eFragment,
		.module = shaderModule,
		.pName = "fragMain"
	};

	vk::PipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreationInfo,fragmentShaderCreationInfo };


	// Niektóre dane moga być przekazywane dynamicznie, co oznacza że należy ja przekonać do PipelineDynamicState ( i nie trzeba przebudowywać całego
	// pipeline przy zmianie tych danych - np rozmiar okna )
	std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

	vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data()
	};

	// wertex Input Create Info -> służy do opisu formatu w jakim przekazywane będa vertexy do vertex shadera
	// są dwie opcje przekazywania - Bindings i Attibute descriptions.  W tym momencie zahardkodowaliśmy vertexy w pixel shader
	// więc tworzymy po prostu
	vk::PipelineVertexInputStateCreateInfo  vertexInputInfo;

	// InputAssembly określa: jaki rodzaj geometrii będzie rysowany z vertexów i czy primitive restart jest enabled czy nie
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{ .topology = vk::PrimitiveTopology::eTriangleList };


	// Viewport -  określa region framebuffera do którego będzie renderowany output ( praktycznie zawsze ma rozmiar (0,0) - (width,height)
	// ostatnie 0.0f, 1.0f - to minDepth i maxDepth ( czyli to co się pojawia w depth buffer - standardowo zakres 0.0 do 1.0 )
	vk::Viewport viewport{ 0.0f, 0.0f, static_cast<float>(InSwapChain->swapChainExtent.width), static_cast<float>(InSwapChain->swapChainExtent.height), 0.0f, 1.0f };

	// Scissors - określają wycinek który ma być renderowany - pixele które nie mieszczą się w ramach rectangla ze scissorsów są po prostu 
	// ignorowane - jeśli chcemy renderować cały Viewport - to ustawiamy scissorsy tak aby pokrywały caly viewport
	vk::Rect2D scissor{ vk::Offset2D{0,0}, InSwapChain->swapChainExtent };
	// Viewport i scissors mogą być używane z dynamic state, a więc mogą być ustawiane dynamicznie.

	// Później ilość viewportów i scissorów i viewporta i scissorsa przypisujemy do PipelineViewportStateCreateInfo
	vk::PipelineViewportStateCreateInfo viewportState{
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor
	};

	// Rasterizer !
	// Rasterizer przekształca geometrie z vertex shader we fragmenty które będą pokolorowane przez fragment shader.
	// Robi depth test, face culling i scissors test. Można też ustawić czy zwraca fragmenty któe pokrywają cały polygon, czy tylko krawędzie (renderowanie wireframe)
	vk::PipelineRasterizationStateCreateInfo rasteriser{
		.depthClampEnable = false,  // <-, depthClamp jest użyteczne dla generowania shadowMaps. Użycie tego wymaga włączenia GPU feature
		.rasterizerDiscardEnable = false,  // <- jeśli true to powoduje że geometria nie przechodzi przez rasterizer
		.polygonMode = vk::PolygonMode::eFill,  // <- użycie innego mode niż eFill wymaga włączenia GPU feature
		.cullMode = vk::CullModeFlagBits::eBack, // <- można włączyć, wyłączyć, albo ustawić na back/front
		.frontFace = vk::FrontFace::eClockwise,  // <- wskazuje która kolejność wierzchołków będzie ustalać czy jest front czy backface
		.depthBiasEnable = vk::False,  // <- nie wiem po chuja vk::False - może być zwykłe false
		.lineWidth = 1.0f  // <- ustawia grubość linii w kontekście ilości fragmentów ( zwiększenie wielkości lineWidth ponad 1.0 wymaga włączenia wideLines GPU feature )
		// <- lineWidth jest używane gdy renderujemy wireframe. Jak renderujemy zwykłe eFill to właściwie nie ma znaczenia
	};


	// Multisampling - czyli forma antyaliasingu  - wymaga włączenia featura na GPU - puki co trzymamy wyłączony
	vk::PipelineMultisampleStateCreateInfo multisampling{ .rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False };


	// Color blenging - po tym jak fragment shader zwróci kolor, ten kolor musi być w jakiś sposób połaczony z kolorem który jest we framebuffer ( właściwie to w render targecie. Sam frame buffer nie przechowuje koloru - Kolor przechowuje Image)
	// - można albo zrobić Mix, albo bitwise operation

	// vk::PipelineColorBlendAttachementState - przechowuje konfigurację per framebuffer
	vk::PipelineColorBlendAttachmentState colorBlendAttachement{
		.blendEnable = vk::False,

		// a jeśli zrobimy blendEnalbe = true, to musimy ustawić inne parametry:
		/*
		.blendEnable = vk::True,
		.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
		.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
		.colorBlendOp        = vk::BlendOp::eAdd,
		.srcAlphaBlendFactor = vk::BlendFactor::eOne,
		.dstAlphaBlendFactor = vk::BlendFactor::eZero,
		.alphaBlendOp        = vk::BlendOp::eAdd,
		*/

		.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
	};

	// vk::PipelineColorBlendStateCreateInfo  <- zawiera globalne ustawienia color blendingu -> przekazujemy do niego też 
	// utworony wcześniej PipelineColorBlendAttachmentState  ( można przekazać wiele )
	// Generalnie z blendingiem kolorów chodzi o to jak np kolor z nowego renderu będzie mieszany ze starym. 
	// Najczęstszym blendingiem jest alpha blending np:    
	// finaColor.rgb =  alpha * newColor + (1-alpha)* oldColor
	// finalColor.a  =  newAlpha.a

	vk::PipelineColorBlendStateCreateInfo colorBlending{
		.logicOpEnable = vk::False,
		.logicOp = vk::LogicOp::eCopy,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachement
	};


	// Pipeline Layout -> pozwala na przekazywanie dynamicznych wartości do shaderów -> Puki co nie będziemy używać, ale i tak musimy utworzyć
	// pusty obiekt i pole w klasie HelloTriangleApp
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{
		.setLayoutCount = 0,
		.pushConstantRangeCount = 0
	};

	pipelineLayout = vk::raii::PipelineLayout(InContext->logicalDevice, pipelineLayoutCreateInfo);

	// Pipeline
	vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo
	{
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &InSwapChain->swapChainSurfaceFormat.format
	};

	vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain
	{
		vk::GraphicsPipelineCreateInfo{
			.stageCount = 2,
			.pStages = shaderStages,
			.pVertexInputState = &vertexInputInfo,
			.pInputAssemblyState = &inputAssembly,
			.pViewportState = &viewportState,
			.pRasterizationState = &rasteriser,
			.pMultisampleState = &multisampling,
			.pColorBlendState = &colorBlending,
			.pDynamicState = &dynamicStateCreateInfo,
			.layout = pipelineLayout,
			.renderPass = nullptr
		},

		vk::PipelineRenderingCreateInfo
		{
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &InSwapChain->swapChainSurfaceFormat.format
		}
	};

	pipeline = vk::raii::Pipeline(InContext->logicalDevice, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
}

std::vector<uint32_t> Vk_Pipeline::ReadFile(const std::string& filename)
{
	// std::ios::ate powoduje że otwieramy plik i od razu przechodzimy na jego koniec - pozwala to od razu sprawdzić rozmiar pliku.
	// a binary - no to że plik czytany binarnie.
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	// Coś było z budowaniem shaderów !

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file");
	}

	// tellg() odczytuje aktualną pozcję "kursora odczytu". Skoro nasz vector przyjmuje <char> czyli bajty, a pozycja odczytu jest podawana w bajtach
	// to pozycja odczytu jest rozmiarem naszego vectora  ( tu zmiana, bo zmieniliśmy char na uint32_t, więc musimy podzielić ilość bajtów
	// przez rozmiar uint32_t które są potrzebne do 
	std::vector<uint32_t> buffer(file.tellg() / sizeof(uint32_t));

	file.seekg(0, std::ios::beg);

	// read zapisuje do 1-arg buffer.data() zwraca wskaźnik na początek bufora, drugi argument to rozmiar ile ma czytać - nasz bufor ma rozmiar pliku
	// i tyle ma czytać ( castujemy rozmiar na std::streamsize)   // reinterpret_cast<char*> bo read obsługuje tylko char.
	file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size() * sizeof(uint32_t)));

	file.close();

	return buffer;
}

vk::raii::ShaderModule Vk_Pipeline::CreateShaderModule(const std::vector<uint32_t>& shaderBytes, Vk_Context* InContext)
{
	vk::ShaderModuleCreateInfo createInfo
	{
		.codeSize = shaderBytes.size() * sizeof(uint32_t),
		// sla shaderModule wskaźnik który przekazujemy powinien być wskaźnikiem na uint32_t a nie na char, więc go reinterpret castujemy
		.pCode = shaderBytes.data()
		// robiąc reinterpret cast, musimy być pewni że dane spałniają wymagania alignmentu ( szczęśliwie std::vector zajmuje się tym )
	};

	// UWAGA - tu jest jakaś nieścisłość z alignmentem - bo vector zapewnia alignment ale dla char, a nie uint32_t - czyli lepiej by było przechowywać
	// kod bitowy w formie uint32_t
	return vk::raii::ShaderModule{ InContext->logicalDevice, createInfo };
}
