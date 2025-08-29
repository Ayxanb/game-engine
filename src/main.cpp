#include <engine.hpp>
#include <core/application.hpp>

#include <ecs/ecs.hpp>
#include <ecs/components.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

/* ----------------- Game Class ----------------- */
class MyGame : public Engine::Application {
  Engine::ECS::EntityManager emanager;

  bool camera_free_move = false;

  glm::vec3 camera_last_non_free_position;
  glm::vec3 camera_last_non_free_orientation;

  glm::vec3 camera_last_free_position;
  glm::vec3 camera_last_free_orientation;

public:
  explicit MyGame(Engine::Instance &_instance) noexcept : Application(_instance) {}

  bool onInit() override;
  bool onTick(float) override;
  bool onUpdate() override;
  bool onRender() override;
};

/* ----------------- Application Entry ----------------- */
int main() {
  Engine::Config::Logger logger_config;

  Engine::Config::Window window_config {
    .title = "Engine",
    .width = 800,
    .height = 600,
    .centered = true,
  };

  Engine::Config::Renderer renderer_config {
    .backend = Engine::GraphicsAPI::Backend::Vulkan,
    .shader_paths = {
      {
        "C:\\Users\\ayxan_5axucza\\source\\repos\\GameEngine\\shaders\\block_vert.spv",
        "C:\\Users\\ayxan_5axucza\\source\\repos\\GameEngine\\shaders\\block_frag.spv",
        // "C:\\Users\\ayxan_5axucza\\source\\repos\\GameEngine\\pipelines\\block.vert",
        // "C:\\Users\\ayxan_5axucza\\source\\repos\\GameEngine\\pipelines\\block.frag",
      },
    },
  };

  Engine::Config::Camera camera_config {
    .near = 0.1f,
    .far = 1000.0f,
    .fov = 90.0f,
  };

  Engine::Config engine_config {
    .logger = logger_config,
    .window = window_config,
    .renderer = renderer_config,
    .camera = camera_config,
  };

  Engine::Instance engine;

  if (!engine.init(engine_config))
    return EXIT_FAILURE;

  MyGame game(engine);
  engine.run(game);
}

/* ----------------- Game Implementation ----------------- */
bool MyGame::onInit() {
  auto &renderer = instance.getRenderer();
  auto &camera   = instance.getCamera();

  // Setup camera
  camera.setPosition({0.0f, 20.0f, 20.0f});
  camera.setOrientation({glm::radians(45.0f), glm::radians(-45.0f), -1.0f});

  File::Path path = "assets\\models\\test.obj";

  // Upload test mesh
  auto mesh = Engine::Mesh::fromOBJ(path);

  if(!mesh){
    LOG_ERROR("unable to load model from file `{}`", path.filename().string());
    return false;
  }
  
  Engine::Mesh::Handle mesh_handle = renderer.addMesh(mesh.value());
  emanager.create<Engine::ECS::Component::Mesh, Engine::ECS::Component::Material>(
    Engine::ECS::Component::Mesh {.handle = mesh_handle},
    Engine::ECS::Component::Material {.handle = 0}
  );

  // Save initial camera states
  camera_last_non_free_position    = camera.getPosition();
  camera_last_non_free_orientation = camera.getOrientation();

  camera_last_free_position        = camera.getPosition();
  camera_last_free_orientation     = camera.getOrientation();

  return true;
}

bool MyGame::onTick(float delta_time) {
  auto &window = instance.getWindow();
  auto &camera = instance.getCamera();

  constexpr float MOVE_SPEED       = 10.0f;
  constexpr float MOUSE_SENSITIVITY = 75.0f;

  if (!camera_free_move) return true;

  // --- Keyboard movement ---
  glm::vec3 orientation = camera.getOrientation();
  glm::vec3 position    = camera.getPosition();

  if (window.isKeyHeld(GLFW_KEY_W)) position += orientation * MOVE_SPEED * delta_time;
  if (window.isKeyHeld(GLFW_KEY_S)) position -= orientation * MOVE_SPEED * delta_time;

  glm::vec3 right = glm::cross(orientation, {0, 1, 0});
  if (window.isKeyHeld(GLFW_KEY_A)) position -= right * MOVE_SPEED * delta_time;
  if (window.isKeyHeld(GLFW_KEY_D)) position += right * MOVE_SPEED * delta_time;

  camera.setPosition(position);

  // --- Mouse rotation ---
  glm::uvec2 size = window.getSize();
  glm::uvec2 mouse_pos = window.getMousePosition();

  float rX = MOUSE_SENSITIVITY * (size.y / 2.0f - mouse_pos.y) / size.y;
  float rY = MOUSE_SENSITIVITY * (size.x / 2.0f - mouse_pos.x) / size.x;

  glm::vec3 rotated = glm::rotate(
    glm::rotate(orientation, glm::radians(rX), glm::normalize(glm::cross(orientation, Engine::Camera::UP))),
    glm::radians(rY), Engine::Camera::UP
  );
  camera.setOrientation(rotated);

  window.setMousePosition({size.x / 2.0f, size.y / 2.0f});
  return true;

}

bool MyGame::onUpdate() {
  auto &window = instance.getWindow();
  auto &camera = instance.getCamera();

  if (!window.isKeyPressed(GLFW_KEY_ESCAPE))
    return true;

  camera_free_move = !camera_free_move;

  if (camera_free_move) {
    camera_last_non_free_position    = camera.getPosition();
    camera_last_non_free_orientation = camera.getOrientation();

    camera.setPosition(camera_last_free_position);
    camera.setOrientation(camera_last_free_orientation);
  }
  else {
    camera_last_free_position        = camera.getPosition();
    camera_last_free_orientation     = camera.getOrientation();

    camera.setPosition(camera_last_non_free_position);
    camera.setOrientation(camera_last_non_free_orientation);
  }

  return true;
}

bool MyGame::onRender() {
  auto &renderer = instance.getRenderer();

  for (auto [entity, mesh, material] : emanager.view<
                                                                          Engine::ECS::Component::Mesh,
                                                                          Engine::ECS::Component::Material>()) {
    if(!renderer.bindPipeline(material.handle) || !renderer.render(mesh.handle))
      return false;
  }

  return true;
}