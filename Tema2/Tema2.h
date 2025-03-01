#pragma once

#include "components/simple_scene.h"
#include "lab_m1/Tema2/lab_camera.h"

namespace m1
{
    class Tema2 : public gfxc::SimpleScene
    {
    public:
        struct ViewportArea
        {
            ViewportArea() : x(0), y(0), width(1), height(1) {}
            ViewportArea(int x, int y, int width, int height)
                : x(x), y(y), width(width), height(height) {}
            int x;
            int y;
            int width;
            int height;
        };

        Tema2();
        ~Tema2();

        void Init() override;

    private:

        void CreateTeren();

        void CreateTree(const std::string& name);
        void GenerateTrees(int count);

        void CreateBuilding(const std::string& name);
        void GenerateBuildings(int count);

        void CreateDrone();
        void CreateCube(const std::string& name, float size, glm::vec3 color);
        void CreateElice(const std::string& name, float width, float height, glm::vec3 color);

		void CreateParaVerti(const std::string& name, float distantaCenter, float lungime, float grosime, glm::vec3 culoareCorp, float start_inaltime);
        void CreateParaOrizont(const std::string& name, float distantaCenter, float grosime, glm::vec3 culoareCorp, float inaltime);
		void CreateCheckPoint();
        void GenerateCheckPoint(int count);

        void CreateArrowMesh();
        void RenderArrow(glm::vec3 position, glm::vec3 direction);

        bool CheckColiziuneCilindru(const glm::vec3& dronePos, const glm::vec3& objectPos, float radius, float height);
        bool CheckColiziuneBox(const glm::vec3& dronePos, const glm::vec3& obstacleCenter, const glm::vec3& droneDimensions, const glm::vec3& obstacleDimensions);
        bool VerificaCheckPoint();

        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;

        void RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, float noise, const glm::vec3& color);
        void RenderUmbraMesh(Mesh* mesh, Shader* shadowShader, const glm::mat4& modelMatrix, const glm::vec3& shadowColor, const glm::vec3& lightDirection);
        Mesh* CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices);
        Mesh* CreateMeshLine(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices);

        void CreateTimer(const std::string& name, float width, float height, glm::vec3 color);
        void CreateTimerContur(const std::string& name, float width, float height, glm::vec3 color);

        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        void RenderScene(float deltaTimeSeconds);
    protected:
        glm::mat4 modelMatrix;
        GLenum polygonMode;
        ViewportArea miniViewportArea;

		std::vector<glm::vec3> treePositions; // pozitiile arborilor
		std::vector<glm::vec3> buildingPositions; // pozitiile cladirilor
		std::vector<glm::vec3> check_point_position; // pozitiile checkpoint-urilor

		float bodyWidth; // lungimea dronei
		float bodyDepth; // grosimea dronei
		float bodyHeight;  // inaltimea dronei
		glm::vec3 dronePosition; // pozitia dronei
		glm::vec3 droneRotation2; // rotatia dronei
        glm::vec3 CameraRotation;

		float propellerRotation; // rotatia elicei
        float armLength2;

        glm::vec3 culoareCorpMic;



        float index;
		int nr_checkpoints;
		std::vector<bool> CheckPoints;

        float val_z_perspectiva;
        bool perspectiva_drona;
        float time_secunde;


        implemented::CameraLab* camera;
    };
}   // namespace m1

