// Core Classes
class Scene {
    std::vector<Node*> nodes;
    Camera* activeCamera;
    void render();
};

class Node {
    glm::mat4 transform;
    std::vector<Node*> children;
    void add(Node* child);
    void updateTransform();
};

class Mesh : public Node {
    Geometry* geometry;
    Material* material;
};

class Camera : public Node {
    glm::mat4 projectionMatrix;
    glm::mat4 getViewMatrix();
};

// Renderer
class Renderer {
    VulkanContext context;
    void renderScene(Scene* scene);
};

// Main Loop
int main() {
    Renderer renderer;
    Scene scene;

    // Setup scene
    Camera* camera = new PerspectiveCamera(75.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Mesh* mesh = new Mesh(geometry, material);
    scene.add(camera);
    scene.add(mesh);

    // Render loop
    while (running) {
        renderer.renderScene(&scene);
    }
}