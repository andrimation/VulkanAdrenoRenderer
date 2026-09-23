#pragma once

#include "glm/glm.hpp"
#include "../VulkanCommon.h"
#include <array>

struct Vertex
{
	glm::fvec2 pos;
	glm::fvec3 color;

    // Poniżej implementacja sposobu doczytu danych wierzchołków przez shader <- coś jak VertexFactory w Unrealu
    
    // 1) Ta funkcja opisuje jak shader ma się poruszać pomiędzy kolejnymi obiektami typu Vertex  
    static vk::VertexInputBindingDescription getBindingDescription()
    {
        return vk::VertexInputBindingDescription{ 
            .binding = 0, // <- określa intex w tablicy powiązań - czyli później getAttributeDescription() również wskazujemy binding 0 to wiadomo że odnoszą się właśnie do tego
            .stride = sizeof(Vertex), // <- określa co ile bajtów zaczyna się nowy obiekt
            .inputRate = vk::VertexInputRate::eVertex  // określa że przechodzi do następnego wpisu danych co każdy vertex ( może być też co eInstance )
        };
    }

    // 2) Ta funkcja określa jak shader ma odczytywać dane w ramach jednego vertexu
    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescription()
    {
        return
        {   // position
            vk::VertexInputAttributeDescription{
                .location = 0, // <- pozycja elementu w strukturze ( w tyn przypadku Vertexa ) 
                .binding = 0,  //
                .format = vk::Format::eR32G32Sfloat, // <- odpowiada to fvec2 - czyli dwa pola 32 bity ( 2 pola float )
                .offset = offsetof(Vertex,pos)  // <- kolejne pole zaczyna się + offsetof(Vertex,pos)
            },
            // color
            vk::VertexInputAttributeDescription{
                .location = 1, // <- pozycja elementu w strukturze ( w tyn przypadku Vertexa ) 
                .binding = 0,  //
                .format = vk::Format::eR32G32B32Sfloat, // <- odpowiada to fvec3 - czyli trzy pola 32 bity ( 3 pola float )
                .offset = offsetof(Vertex,color)  // <- kolejne pole zaczyna się + offsetof(Vertex,pos)
            }
        };
    }
};

// Poniżej -> interleaving vertex attributes - czyli sytuacja kiedy mamy vector/array vertexów które trzymają pozycje, color itp
// - jako przeciwieństwo tego można mieć osobne array/vectory pozycji, colorów itp
const std::vector<Vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

