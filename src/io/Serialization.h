#pragma once

#include <nlohmann/json.hpp>
#include <SFML/Graphics.hpp>
#include "level/Level.h"
#include "project/AssetManager.h"
#include "project/Project.h"

namespace sf
{
	void to_json(nlohmann::json& j, const sf::Vector2f& vec)
	{
		j = nlohmann::json{ {"x", vec.x}, {"y", vec.y} };
	}
	void from_json(const nlohmann::json& j, sf::Vector2f& vec)
	{
		j.at("x").get_to(vec.x);
		j.at("y").get_to(vec.y);
	}

	void to_json(nlohmann::json& j, const sf::Color& color)
	{
		j = nlohmann::json{
			{"r", color.r },
			{"g", color.g },
			{"b", color.b },
			{"a", color.a }
		};
	}
	void from_json(const nlohmann::json& j, sf::Color& color)
	{
		j.at("r").get_to(color.r);
		j.at("g").get_to(color.g);
		j.at("b").get_to(color.b);
		j.at("a").get_to(color.a);
	}

	void to_json(nlohmann::json& j, const sf::Vertex& ver)
	{
		j = nlohmann::json{
			{"color", ver.color},
			{"position", ver.position}
		};
	}
	void from_json(const nlohmann::json& j, sf::Vertex& ver)
	{
		j.at("color").get_to(ver.color);
		j.at("position").get_to(ver.position);
	}

	void to_json(nlohmann::json& j, const sf::PrimitiveType& prim)
	{
		switch (prim)
		{
		case sf::PrimitiveType::Points:			j = "Points";			break;
		case sf::PrimitiveType::Lines:			j = "Lines";			break;
		case sf::PrimitiveType::LineStrip:		j = "LineStrip";		break;
		case sf::PrimitiveType::Triangles:		j = "Triangles";		break;
		case sf::PrimitiveType::TriangleStrip:	j = "TriangleStrip";	break;
		case sf::PrimitiveType::TriangleFan:	j = "TriangleFan";		break;
		}
	}
	void from_json(const nlohmann::json& j, sf::PrimitiveType& prim)
	{
		const std::string primStr = j.get<std::string>();
		if (primStr == "Points")				prim = sf::PrimitiveType::Points;
		else if (primStr == "Lines")			prim = sf::PrimitiveType::Lines;
		else if (primStr == "LineStrip")		prim = sf::PrimitiveType::LineStrip;
		else if (primStr == "Triangles")		prim = sf::PrimitiveType::Triangles;
		else if (primStr == "TriangleStrip")	prim = sf::PrimitiveType::TriangleStrip;
		else if (primStr == "TriangleFan")		prim = sf::PrimitiveType::TriangleFan;
		else {
			throw std::invalid_argument("Invalid sf::PrimitiveType string");
		}
	}

	void to_json(nlohmann::json& j, const sf::VertexArray& vertexArray)
	{
		j = nlohmann::json{
			{"primitiveType", vertexArray.getPrimitiveType()},
			{"vertices", nlohmann::json::array()}
		};
		auto& vertices = j.at("vertices");
		for (size_t i = 0; i < vertexArray.getVertexCount(); i++)
		{
			vertices.push_back(vertexArray[i]);
		}
	}
	void from_json(const nlohmann::json& j, sf::VertexArray& vertexArray)
	{
		vertexArray.setPrimitiveType(j.at("primitiveType").get<sf::PrimitiveType>());
		vertexArray.clear();
		auto& vertices = j.at("vertices");
		for (const auto& vertex : vertices)
		{
			vertexArray.append(vertex.get<sf::Vertex>());
		}
	}

	void to_json(nlohmann::json& j, const sf::Angle& angle)
	{
		j = nlohmann::json{
			{"degrees", angle.asDegrees()}
		};
	}
	void from_json(const nlohmann::json& j, sf::Angle& angle)
	{
		float degrees = j.at("degrees").get<float>();
		angle = sf::degrees(degrees);
	}

}

namespace nlohmann {
	template <typename T>
	struct adl_serializer<std::unique_ptr<T>> {
		static void to_json(json& j, const std::unique_ptr<T>& ptr) {
			if (ptr) {
				j = *ptr;
			}
			else {
				j = nullptr;
			}
		}

		static void from_json(const json& j, std::unique_ptr<T>& ptr) {
			if (j.is_null()) {
				ptr = nullptr;
			}
			else {
				ptr = std::make_unique<T>();
				j.get_to(*ptr);
			}
		}
	};
}

namespace vle {
	void to_json(nlohmann::json& j, const GameObject& object)
	{
		j = nlohmann::json{
			{"assetID", object.assetID},
			{"position", object.sprite->getPosition()},
			{"scale", object.sprite->getScale()},
			{"rotation", object.sprite->getRotation()},
			{"origin", object.sprite->getOrigin()}
		};
	}
	void from_json(const nlohmann::json& j, GameObject& object)
	{
		j.at("assetID").get_to(object.assetID);
		const sf::Texture texture;
		object.sprite.emplace(texture);
		object.sprite->setPosition(j.at("position").get<sf::Vector2f>());
		object.sprite->setScale(j.at("scale").get<sf::Vector2f>());
		object.sprite->setRotation(j.at("rotation").get<sf::Angle>());
		object.sprite->setOrigin(j.at("origin").get<sf::Vector2f>());
	}

	void to_json(nlohmann::json& j, const Level& level)
	{
		j = nlohmann::json{
			{"levelNameId", level.levelNameId},
			{"hitboxMap", level.hitboxMap},
			{"gameObjects", level.gameObjects}
		};
	}
	void from_json(const nlohmann::json& j, Level& level)
	{
		j.at("levelNameId").get_to(level.levelNameId);
		j.at("hitboxMap").get_to(level.hitboxMap);
		j.at("gameObjects").get_to(level.gameObjects);
	}

	void to_json(nlohmann::json& j, const Asset& asset)
	{
		j = nlohmann::json{
			{"texturePath", asset.texturePath},
			{"defaultScale", asset.defaultScale},
			{"defaultRotation", asset.defaultRotation}
		};
	}
	void from_json(const nlohmann::json& j, Asset& asset)
	{
		j.at("texturePath").get_to(asset.texturePath);
		j.at("defaultScale").get_to(asset.defaultScale);
		j.at("defaultRotation").get_to(asset.defaultRotation);
	}

	void to_json(nlohmann::json& j, const Project& project)
	{
		j = nlohmann::json{
			{"level", project.level},
			{"backgroundTexturePath", project.backgroundTexturePath},
			{"hitboxTexturePath", project.hitboxTexturePath},
			{"simplifyIndex", project.simplifyIndex},
			{"bHitboxMap", project.bHitboxMap},
			{"assets", project.assets}
		};
	}
	void from_json(const nlohmann::json& j, Project& project)
	{
		j.at("level").get_to(project.level);
		j.at("backgroundTexturePath").get_to(project.backgroundTexturePath);
		j.at("hitboxTexturePath").get_to(project.hitboxTexturePath);
		j.at("simplifyIndex").get_to(project.simplifyIndex);
		j.at("bHitboxMap").get_to(project.bHitboxMap);
		j.at("assets").get_to(project.assets);
	}
}