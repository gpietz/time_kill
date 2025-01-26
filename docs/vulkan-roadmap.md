# Vulkan Roadmap
___

### 1. Create the Swapchain

The swapchain is responsible for managing the images that will be presented to the screen. 
It allows you to render to one image while presenting another.
Steps:
* Query swapchain support (formats, present modes, and capabilities).
* Choose the best settings for the swapchain (e.g., surface format, present mode, extent).
* Create the swapchain using vkCreateSwapchainKHR.

Key Functions:
 * vkGetPhysicalDeviceSurfaceCapabilitiesKHR
 * vkGetPhysicalDeviceSurfaceFormatsKHR
 * vkGetPhysicalDeviceSurfacePresentModesKHR
 * vkCreateSwapchainKHR
___

### 2. Create Image Views

Each image in the swapchain needs an image view to be used in rendering. Image views describe how the image should be accessed (e.g., as a 2D texture).
Steps:
* Retrieve the swapchain images using vkGetSwapchainImagesKHR.
* Create an image view for each swapchain image using vkCreateImageView.

Key Functions:
* vkGetSwapchainImagesKHR
* vkCreateImageView
___

### 3. Create a Render Pass

The render pass defines the structure of your rendering pipeline, including:

* The format of the framebuffer attachments (e.g., color, depth).
* How the attachments are used (e.g., cleared, stored).
* Subpasses and dependencies between them.

Steps:
* Define the color and depth attachments.
* Create the render pass using vkCreateRenderPass.

Key Functions:
* vkCreateRenderPass
___
### 4. Create the Graphics Pipeline

The graphics pipeline describes how rendering commands are processed. It includes:
* Shader stages (vertex, fragment, etc.).
* Vertex input layout.
* Rasterization settings.
* Color blending.

Steps:
* Load shaders (e.g., vertex and fragment shaders) and create shader modules.
* Define the pipeline layout (e.g., push constants, descriptor sets).
* Create the graphics pipeline using vkCreateGraphicsPipelines.

Key Functions:
* vkCreateShaderModule
* vkCreatePipelineLayout
* vkCreateGraphicsPipelines
___
### 5. Create Framebuffers

Framebuffers are used to bind the swapchain images to the render pass. Each framebuffer corresponds to a swapchain image.
Steps:
1. Create a framebuffer for each swapchain image using vkCreateFramebuffer.

Key Functions:
* vkCreateFramebuffer
___
### 6. Create Command Buffers

Command buffers are used to record rendering commands (e.g., binding pipelines, drawing).
Steps:
1. Create a command pool using vkCreateCommandPool.
2. Allocate command buffers using vkAllocateCommandBuffers.
3. Record commands into the command buffers (e.g., begin render pass, draw, end render pass).

Key Functions:
* vkCreateCommandPool
* vkAllocateCommandBuffers
* vkBeginCommandBuffer
* vkCmdBeginRenderPass
* vkCmdBindPipeline
* vkCmdDraw
* vkCmdEndRenderPass
* vkEndCommandBuffer
___
### 7. Set Up Synchronization
Synchronization ensures that the CPU and GPU work together correctly. You’ll need:
* Semaphores: For synchronizing between GPU operations (e.g., image acquisition, rendering, presentation).
* Fences: For synchronizing between the CPU and GPU.

Steps: 
1. Create semaphores and fences using vkCreateSemaphore and vkCreateFence.
2. Use them in the rendering loop to synchronize operations.

Key Functions:
* vkCreateSemaphore
* vkCreateFence
* vkAcquireNextImageKHR
* vkQueueSubmit
* vkQueuePresentKHR
___
### 8. Implement the Rendering Loop

The rendering loop is where you:
1. Acquire the next image from the swapchain.
2. Submit command buffers to the graphics queue.
3. Present the image to the screen.

Steps:
1. Use vkAcquireNextImageKHR to acquire an image.
2. Use vkQueueSubmit to submit the command buffer.
3. Use vkQueuePresentKHR to present the image.

Key Functions:
* vkAcquireNextImageKHR
* vkQueueSubmit
* vkQueuePresentKHR
___
### 9. Clean Up Resources
Ensure that all Vulkan objects are properly destroyed when your application exits. This includes:
* Swapchain and image views.
* Render pass and graphics pipeline.
* Framebuffers and command buffers.
* Semaphores and fences.
* Logical device and instance.

Key Functions:
* vkDestroySwapchainKHR
* vkDestroyImageView
* vkDestroyRenderPass
* vkDestroyPipeline
* vkDestroyFramebuffer
* vkFreeCommandBuffers
* vkDestroyCommandPool
* vkDestroySemaphore
* vkDestroyFence
* vkDestroyDevice
* vkDestroyInstance
