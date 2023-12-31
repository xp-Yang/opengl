#include "SceneHierarchy.hpp"
#include "GamePlay/Framework/ECS/Components.hpp"
#include "ResourceManager/MyModel.hpp"
#include "ResourceManager/Texture.hpp"
#include "Platform/RHI/rhi.hpp"

SceneHierarchy::SceneHierarchy() {
	init();
}

void SceneHierarchy::init() {
	std::string resource_dir = RESOURCE_DIR;

    // TODO 这些需要被SceneHierarchy管理吗，参考filament
	auto& world = ecs::World::get();

	auto skybox_entity = world.create_entity();
	world.addComponent<ecs::NameComponent>(skybox_entity).name = "skybox";
	world.addComponent<ecs::TransformComponent>(skybox_entity);
	auto& skybox_renderable = world.addComponent<ecs::RenderableComponent>(skybox_entity);
	ecs::Primitive skybox_primitive;
	skybox_primitive.mesh = Mesh::create_cube_mesh();
	std::vector<std::string> faces
	{
		resource_dir + "/images/skybox/right.jpg",
		resource_dir + "/images/skybox/left.jpg",
		resource_dir + "/images/skybox/top.jpg",
		resource_dir + "/images/skybox/bottom.jpg",
		resource_dir + "/images/skybox/front.jpg",
		resource_dir + "/images/skybox/back.jpg"
	};
	// TODO 是否可以放到material中
	// TODO component size crash
	world.addComponent<ecs::SkyboxComponent>(skybox_entity).texture = Texture::generate_cube_map(faces);
	Material skybox_material;
	skybox_material.shader = new Shader(resource_dir + "/shader/skybox.vs", resource_dir + "/shader/skybox.fs");
	skybox_primitive.material = skybox_material;
	skybox_renderable.setPrimitives({ skybox_primitive });

	//auto light_entity = world.create_entity();
	//world.addComponent<ecs::NameComponent>(light_entity).name = "light";
 //   world.addComponent<ecs::LightComponent>(light_entity);
	//auto& light_transform = world.addComponent<ecs::TransformComponent>(light_entity);
 //   light_transform.translation = { 12.0f, 12.0f, 0.0f };
 //   light_transform.scale = Vec3(0.5f);
	//auto& light_renderable = world.addComponent<ecs::RenderableComponent>(light_entity);
	//ecs::Primitive light_primitive;
	//light_primitive.mesh = Mesh::create_cube_mesh();
	//Material light_material;
	//light_material.color = {255.f / 255.0f, 255.f / 255.0f, 175.f / 255.0f, 175.f / 255.0f };
	//light_material.shader = new Shader(resource_dir + "/shader/light.vs", resource_dir + "/shader/light.fs");
	//light_primitive.material = light_material;
	//light_renderable.setPrimitives({ light_primitive });

	//auto directional_light_entity = world.create_entity();
	//world.addComponent<ecs::NameComponent>(directional_light_entity).name = "directional_light";
	//world.addComponent<ecs::LightComponent>(directional_light_entity);
	//auto& directional_light_transform = world.addComponent<ecs::TransformComponent>(directional_light_entity);
	//directional_light_transform.translation = { 0.0f, 15.0f, 0.0f };
	//directional_light_transform.scale = Vec3(0.5f);
	//auto& directional_light_renderable = world.addComponent<ecs::RenderableComponent>(directional_light_entity);
	//ecs::Primitive directional_light_primitive;
	//directional_light_primitive.mesh = Mesh::create_cube_mesh();
	//Material directional_light_material;
	//directional_light_material.shader = new Shader(resource_dir + "/shader/light.vs", resource_dir + "/shader/light.fs");
	//directional_light_primitive.material = directional_light_material;
	//directional_light_renderable.setPrimitives({ directional_light_primitive });

	static const int LIGHT_COUNT = 3;
	static Shader* light_shader = new Shader(resource_dir + "/shader/light.vs", resource_dir + "/shader/light.fs");
	for (int i = 0; i < LIGHT_COUNT; i++) {
		auto light_entity = world.create_entity();
		world.addComponent<ecs::NameComponent>(light_entity).name = std::string("light") + std::to_string(i);
		world.addComponent<ecs::LightComponent>(light_entity);
		auto& light_transform = world.addComponent<ecs::TransformComponent>(light_entity);
		srand(i * i * i * i + 5363 * i * i * i - 251 * i * i + 6455);
		double r = (rand() / double(RAND_MAX) - 0.5) * 2;
		srand(- i * i * i + 73425 * i * i - 97825 * i + 12532);
		double r2 = rand() / double(RAND_MAX);
		srand(i * i * i * i * i + 2351425 * i * i * i - 1522532);
		double r3 = (rand() / double(RAND_MAX) - 0.5) * 2;
		light_transform.translation = { 12.0f * r, 10.0f * r2, 12.0f * r3 };
		light_transform.scale = Vec3(0.2f);
		auto& light_renderable = world.addComponent<ecs::RenderableComponent>(light_entity);
		auto& light_properties = world.addComponent<ecs::LightComponent>(light_entity);
		ecs::Primitive light_primitive;
		//light_primitive.mesh = Mesh::create_cube_mesh();
		light_primitive.mesh = Mesh::create_icosphere_mesh(5);
		light_properties.color = { 255.f * std::abs(r) / 255.0f, 255.f * std::abs(r2) / 255.0f, 255.f * std::abs(r3) / 255.0f, 175.f / 255.0f };
		if (LIGHT_COUNT >= 3)
			light_properties.color /= LIGHT_COUNT / 3;
		else
			light_properties.color = { 1.0f, 1.0f, 1.0f, 1.0f };
		Material light_material;
		light_material.shader = light_shader;
		light_primitive.material = light_material;
		light_renderable.setPrimitives({ light_primitive });
	}

	auto cube_entity = world.create_entity();
	world.addComponent<ecs::NameComponent>(cube_entity).name = "cube";
	auto& cube_transform = world.addComponent<ecs::TransformComponent>(cube_entity);
	cube_transform.translation = { 0.0f, 1.0f, 0.0f };
	auto& cube_renderable = world.addComponent<ecs::RenderableComponent>(cube_entity);
	ecs::Primitive cube_primitive;
	cube_primitive.mesh = Mesh::create_cube_mesh();
	Material cube_material;
    cube_material.shader = new Shader(resource_dir + "/shader/model.vs", resource_dir + "/shader/model.fs", resource_dir + "/shader/model.gs");
	cube_material.set_diffuse_map(resource_dir + "/images/pure_white_map.png");
	cube_material.set_specular_map(resource_dir + "/images/cube_specular.png");
	cube_primitive.material = cube_material;
	cube_renderable.setPrimitives({ cube_primitive });
    world.addComponent<ecs::ExplosionComponent>(cube_entity);

	auto sphere_entity = world.create_entity();
	world.addComponent<ecs::NameComponent>(sphere_entity).name = "sphere";
	auto& sphere_transform = world.addComponent<ecs::TransformComponent>(sphere_entity);
    sphere_transform.translation = { 1.0f, 2.0f, -2.0f };
	auto& sphere_renderable = world.addComponent<ecs::RenderableComponent>(sphere_entity);
	ecs::Primitive sphere_primitive;
	sphere_primitive.mesh = Mesh::create_icosphere_mesh(5);
	Material sphere_material;
	sphere_material.shader = new Shader(resource_dir + "/shader/model.vs", resource_dir + "/shader/model.fs", resource_dir + "/shader/model.gs");
	sphere_material.set_diffuse_map(resource_dir + "/images/pure_white_map.png");
	sphere_material.set_specular_map(resource_dir + "/images/pure_white_map.png");
	sphere_primitive.material = sphere_material;
	sphere_renderable.setPrimitives({ sphere_primitive });
	world.addComponent<ecs::ExplosionComponent>(sphere_entity);

    auto ground_entity = world.create_entity();
    world.addComponent<ecs::NameComponent>(ground_entity).name = "ground";
    auto& ground_transform = world.addComponent<ecs::TransformComponent>(ground_entity);
	ground_transform.scale = Vec3(40.0f);
	auto& ground_renderable = world.addComponent<ecs::RenderableComponent>(ground_entity);
	ecs::Primitive ground_primitive;
	ground_primitive.mesh = Mesh::create_quad_mesh(Point3(-0.5f, 0.0f, 0.5f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f));
    Material ground_material;
    ground_material.shader = new Shader(resource_dir + "/shader/model.vs", resource_dir + "/shader/model.fs", resource_dir + "/shader/model.gs");
    ground_material.set_diffuse_map(resource_dir + "/images/grid.png");
    ground_material.set_specular_map(resource_dir + "/images/grid.png");
	ground_primitive.material = ground_material;
	ground_renderable.setPrimitives({ ground_primitive });

	Model* nanosuit = new Model(resource_dir + "/model/nanosuit/nanosuit.obj");
	auto nanosuit_entity = world.create_entity();
	world.addComponent<ecs::NameComponent>(nanosuit_entity).name = "nanosuit";
	auto& nanosuit_transform = world.addComponent<ecs::TransformComponent>(nanosuit_entity);
	nanosuit_transform.translation = { 5.0f, 0.0f, 0.0f };
	nanosuit_transform.scale = Vec3(0.4f);
	auto& nanosuit_renderable = world.addComponent<ecs::RenderableComponent>(nanosuit_entity);
	std::vector<ecs::Primitive> nanosuit_primitives;
	Shader* nanosuit_shader = new Shader(resource_dir + "/shader/model.vs", resource_dir + "/shader/model.fs", resource_dir + "/shader/model.gs");
	for (int i = 0; i < nanosuit->get_datas().size(); i++) {
		ecs::Primitive primitive;
		primitive.mesh = nanosuit->get_datas().at(i).mesh;
		primitive.material = nanosuit->get_datas().at(i).material;
		primitive.material.shader = nanosuit_shader;
		nanosuit_primitives.push_back(primitive);
	}
	nanosuit_renderable.setPrimitives(nanosuit_primitives);
	world.addComponent<ecs::ExplosionComponent>(nanosuit_entity);

	//Model* yoko = new Model(resource_dir + "/model/yoko/008.obj");
	//auto yoko_entity = world.create_entity();
	//world.addComponent<ecs::NameComponent>(yoko_entity).name = "yoko";
	//auto& yoko_transform = world.addComponent<ecs::TransformComponent>(yoko_entity);
	//yoko_transform.translation = { -5.0f, 0.0f, 0.0f };
	//yoko_transform.scale = Vec3(0.25f);
	//auto& yoko_renderable = world.addComponent<ecs::RenderableComponent>(yoko_entity);
	//std::vector<ecs::Primitive> yoko_primitives;
	//Shader* yoko_shader = new Shader(resource_dir + "/shader/model.vs", resource_dir + "/shader/model.fs", resource_dir + "/shader/model.gs");
	//for (int i = 0; i < yoko->get_datas().size(); i++) {
	//	ecs::Primitive primitive;
	//	primitive.mesh = yoko->get_datas().at(i).mesh;
	//	primitive.material = yoko->get_datas().at(i).material;
	//	primitive.material.shader = yoko_shader;
	//	yoko_primitives.push_back(primitive);
	//}
	//yoko_renderable.setPrimitives(yoko_primitives);
	//world.addComponent<ecs::ExplosionComponent>(yoko_entity);

	Model* bunny = new Model(resource_dir + "/model/bunny.obj");
	auto bunny_entity = world.create_entity();
	world.addComponent<ecs::NameComponent>(bunny_entity).name = "bunny";
	auto& bunny_transform = world.addComponent<ecs::TransformComponent>(bunny_entity);
	bunny_transform.translation = { -5.0f, 0.0f, 0.0f };
	bunny_transform.scale = Vec3(25.0f);
	auto& bunny_renderable = world.addComponent<ecs::RenderableComponent>(bunny_entity);
	std::vector<ecs::Primitive> bunny_primitives;
	Shader* bunny_shader = new Shader(resource_dir + "/shader/model.vs", resource_dir + "/shader/model.fs", resource_dir + "/shader/model.gs");
	for (int i = 0; i < bunny->get_datas().size(); i++) {
		ecs::Primitive primitive;
		primitive.mesh = bunny->get_datas().at(i).mesh;
		primitive.material = bunny->get_datas().at(i).material;
		primitive.material.shader = bunny_shader;
		primitive.material = sphere_material;
		bunny_primitives.push_back(primitive);
	}
	bunny_renderable.setPrimitives(bunny_primitives);
	world.addComponent<ecs::ExplosionComponent>(bunny_entity);

	auto camera = world.create_entity();
	world.addComponent<ecs::NameComponent>(camera).name = "camera";
	auto& camera_component = world.addComponent<ecs::CameraComponent>(camera);
}