#include <core/graphics/vulkan/vkdevice.hpp>
#include <core/graphics/vulkan/vkinstance.hpp>
#include <core/graphics/vulkan/vksurface.hpp>
#include <core/logging.hpp>

#include <cstdint>
#include <set>
#include <span>

namespace Engine {

using Vulkan = GraphicsAPI::Vulkan;

Vulkan::DeviceManager::~DeviceManager() noexcept {
  if (device != VK_NULL_HANDLE) {
    vkDestroyDevice(device, nullptr);
    device = VK_NULL_HANDLE;
    LOG_INFO("[GraphicsAPI::Vulkan::DeviceManager]: Logical device destroyed");
  }
}

bool Vulkan::DeviceManager::init(
  Vulkan::InstanceManager * instance_manager,
  Vulkan::SurfaceManager * surface_manager
) {
  LOG_INFO("[GraphicsAPI::Vulkan::DeviceManager]: Initializing...");
  instance = instance_manager->getInstance();
  surface = surface_manager->getSurface();

  if (!pickPhysicalDevice()) {
    LOG_ERROR("[GraphicsAPI::Vulkan::DeviceManager]: Failed to pick suitable physical device");
    return false;
  }

  if (!createLogicalDevice()) {
    LOG_ERROR("[GraphicsAPI::Vulkan::DeviceManager]: Failed to create logical device");
    return false;
  }

  LOG_INFO("[GraphicsAPI::Vulkan::DeviceManager]: Initialized successfully");
  
  return true;
}

bool Vulkan::DeviceManager::pickPhysicalDevice() {
  uint32_t device_count = 0;
  VkResult result = vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
  
  if (result != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan::DeviceManager]: Failed to enumerate physical devices (Error: {})", (uint32_t)result);
    return false;
  }

  if (device_count == 0) {
    LOG_ERROR("[GraphicsAPI::Vulkan::DeviceManager]: No Vulkan-compatible GPU found!");
    return false;
  }

  std::vector<VkPhysicalDevice> devices(device_count);
  result = vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
  
  if (result != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan::DeviceManager]: Failed to get physical devices (Error: {})", (uint32_t)result);
    return false;
  }

  LOG_INFO("[GraphicsAPI::Vulkan::DeviceManager]: Found {} Vulkan-capable device(s)", device_count);

  for (VkPhysicalDevice &candidate : devices) {
    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(candidate, &props);

    LOG_INFO("[GraphicsAPI::Vulkan::DeviceManager]: Evaluating device: `{}` (Type: {})", 
              props.deviceName, 
              deviceTypeToString(props.deviceType));

    if (!isDeviceSuitable(candidate)) {
      LOG_WARN("[GraphicsAPI::Vulkan::DeviceManager]: Device `{}` is not suitable", props.deviceName);
      continue;
    }

    physical_device = candidate;
    LOG_INFO("[GraphicsAPI::Vulkan::DeviceManager]: Selected physical device: `{}` (Type: {})", 
             props.deviceName, deviceTypeToString(props.deviceType));
                 
    return true;
  }

  LOG_ERROR("[GraphicsAPI::Vulkan::DeviceManager]: No suitable GPU found among {} devices", device_count);
  return false;
}

bool Vulkan::DeviceManager::findQueueFamilies(VkPhysicalDevice device) {
  uint32_t count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

  std::vector<VkQueueFamilyProperties> families(count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

  for (uint32_t i = 0; i < count; ++i) {
    if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphics_queue_family = i;
    }

    VkBool32 present_support = VK_FALSE;
    VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);

    if (result != VK_SUCCESS) {
      LOG_ERROR("[GraphicsAPI::Vulkan::DeviceManager]: Failed to get surface support for queue family {}", i);
      return false;
    }

    if (present_support) {
      present_queue_family = i;
    }

    if (graphics_queue_family.has_value() &&present_queue_family.has_value())
      return true;
  }

  LOG_WARN("[GraphicsAPI::Vulkan::DeviceManager]: Failed to find both graphics and present queue families");
  return false;
}

bool Vulkan::DeviceManager::createLogicalDevice() {
  if (!graphics_queue_family.has_value() || !present_queue_family.has_value()) {
    LOG_ERROR("[GraphicsAPI::Vulkan::DeviceManager]: Missing required queue families");
    return false;
  }

  std::set<uint32_t> unique_families = {
    graphics_queue_family.value(),
    present_queue_family.value()
  };

  float priority = 1.0f;
  std::vector<VkDeviceQueueCreateInfo> queue_infos;

  for (uint32_t family : unique_families) {
    VkDeviceQueueCreateInfo &info = queue_infos.emplace_back();
    info = VkDeviceQueueCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueFamilyIndex = family,
      .queueCount = 1,
      .pQueuePriorities = &priority
    };
  }

  VkPhysicalDeviceFeatures features{};
  vkGetPhysicalDeviceFeatures(physical_device, &features);

  std::vector<const char*> required_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

  if (!checkDeviceExtensionSupport(physical_device, required_extensions)) {
    LOG_ERROR("[GraphicsAPI::Vulkan::DeviceManager]: Required device extensions not supported");
    return false;
  }

  VkDeviceCreateInfo device_info{
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size()),
    .pQueueCreateInfos = queue_infos.data(),
    .enabledLayerCount = 0,
    .ppEnabledLayerNames = nullptr,
    .enabledExtensionCount = static_cast<uint32_t>(required_extensions.size()),
    .ppEnabledExtensionNames = required_extensions.data(),
    .pEnabledFeatures = &features
  };

  VkResult result = vkCreateDevice(physical_device, &device_info, nullptr, &device);
  if (result != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan::DeviceManager]: Failed to create logical device (Error: {})", (uint32_t)result);
    return false;
  }

  vkGetDeviceQueue(device, graphics_queue_family.value(), 0, &graphics_queue);
  vkGetDeviceQueue(device, present_queue_family.value(), 0, &present_queue);

  LOG_INFO("[GraphicsAPI::Vulkan::DeviceManager]: Logical device created successfully");

  return true;
}

bool Vulkan::DeviceManager::checkDeviceExtensionSupport(
  VkPhysicalDevice device, std::span<const char*> required_extensions
) const {
  uint32_t extension_count = 0;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

  std::vector<VkExtensionProperties> available(extension_count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available.data());

  for (const char * ext : required_extensions) {
    bool found = false;
    for (const auto &property : available) {
      if (strcmp(property.extensionName, ext) == 0) {
        found = true;
        break;
      }
    }

    if (!found) {
      LOG_ERROR("[GraphicsAPI::Vulkan::DeviceManager]: Required extension {} not supported", ext);
      return false;
    }
  }

  LOG_INFO("[GraphicsAPI::Vulkan::DeviceManager]: All required extensions are supported");
  return true;
}

std::string_view Vulkan::DeviceManager::deviceTypeToString(VkPhysicalDeviceType type) {
  switch (type) {
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: 
      return "Integrated GPU";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: 
      return "Discrete GPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: 
      return "Virtual GPU";
    case VK_PHYSICAL_DEVICE_TYPE_CPU: 
      return "CPU";
    case VK_PHYSICAL_DEVICE_TYPE_OTHER: 
      return "Other";
    case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
    default:
      return "Unknown";
  }
}

} /* namespace Engine */