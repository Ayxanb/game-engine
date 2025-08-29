#include <core/graphics/vulkan/vkinstance.hpp>
#include <core/platform/window.hpp>
#include <core/logging.hpp>

#include <algorithm>
#include <cstring>

namespace Engine {

using Vulkan = GraphicsAPI::Vulkan;

inline constexpr std::array validation_layers = {
  "VK_LAYER_KHRONOS_validation"
};

Vulkan::InstanceManager::~InstanceManager() noexcept {
  if (debug_messenger) {
    destroyDebugUtilsMessenger(instance, debug_messenger, nullptr);
    debug_messenger = VK_NULL_HANDLE;
  }

  if (instance) {
    vkDestroyInstance(instance, nullptr);
    instance = VK_NULL_HANDLE;
  }

  LOG_INFO("[GraphicsAPI::Vulkan::InstanceManager]: instance destroyed");
}

bool Vulkan::InstanceManager::init(Window* _window, std::string_view app_name, bool enable_validation) {
  window = _window;
  validation_enabled = enable_validation;

  if (validation_enabled && !checkValidationLayerSupport()) {
    LOG_WARN("[GraphicsAPI::Vulkan::InstanceManager]: validation layers requested but not available");
    return false;
  }

  if (!createInstance(app_name))
    return false;

  if (validation_enabled && !setupDebugMessenger())
    return false;

  return true;
}

bool Vulkan::InstanceManager::createInstance(std::string_view app_name) {
  VkApplicationInfo app_info{
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pNext = nullptr,
    .pApplicationName = app_name.data(),
    .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
    .pEngineName = "Custom Engine",
    .engineVersion = VK_MAKE_VERSION(0, 0, 1),
    .apiVersion = VK_API_VERSION_1_4
  };

  std::vector<const char *> extensions = getRequiredExtensions();

  VkInstanceCreateInfo create_info{
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .pApplicationInfo = &app_info,
    .enabledLayerCount = 0,
    .ppEnabledLayerNames = nullptr,
    .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
    .ppEnabledExtensionNames = extensions.data(),
  };

  VkDebugUtilsMessengerCreateInfoEXT debug_info{};
  if (validation_enabled) {
    create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
    create_info.ppEnabledLayerNames = validation_layers.data();
    populateDebugMessengerCreateInfo(debug_info);
    create_info.pNext = &debug_info;
  }

  if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan::InstanceManager]: failed to create instance");
    return false;
  }

  LOG_INFO("[GraphicsAPI::Vulkan::InstanceManager]: instance created successfully");
  return true;
}

bool Vulkan::InstanceManager::checkValidationLayerSupport() const {
  uint32_t layer_count{};
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  return std::ranges::all_of(validation_layers, [&](const char *layer_name) {
    return std::ranges::any_of(available_layers, [&](VkLayerProperties &layer) {
      return std::strcmp(layer_name, layer.layerName) == 0;
    });
  });
}

std::vector<const char*> Vulkan::InstanceManager::getRequiredExtensions() const {
  uint32_t count{};
  const char **extensions_raw = window->getRequiredInstanceExtensions(&count);
  std::vector<const char*> extensions(extensions_raw, extensions_raw + count);

  if (validation_enabled)
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  return extensions;
}

bool Vulkan::InstanceManager::setupDebugMessenger() {
  VkDebugUtilsMessengerCreateInfoEXT create_info{};
  populateDebugMessengerCreateInfo(create_info);

  if (createDebugUtilsMessengerExt(instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan::InstanceManager]: failed to set up debug messenger");
    return false;
  }

  return true;
}

void Vulkan::InstanceManager::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &create_info) {
  create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info.messageType =
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pfnUserCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT,
                                    VkDebugUtilsMessageTypeFlagsEXT,
                                    const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                                    void*) -> VkBool32 {
      (void)callback_data;
      LOG_ERROR("Validation error ({}):\n\t{}", callback_data->pMessageIdName, callback_data->pMessage);
      return VK_FALSE;
  };
}

VkResult Vulkan::InstanceManager::createDebugUtilsMessengerExt(
  VkInstance instance,
  const VkDebugUtilsMessengerCreateInfoEXT* create_info,
  const VkAllocationCallbacks* allocator,
  VkDebugUtilsMessengerEXT* messenger) 
{
  auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
    vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
  return func ? func(instance, create_info, allocator, messenger) : VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Vulkan::InstanceManager::destroyDebugUtilsMessenger(
  VkInstance instance,
  VkDebugUtilsMessengerEXT messenger,
  const VkAllocationCallbacks* allocator)
{
  if (!messenger) return;

  auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
    vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

  if (func)
    func(instance, messenger, allocator);
}

} /* namespace Engine */