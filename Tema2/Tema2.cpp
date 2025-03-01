#include "lab_m1/Tema2/Tema2.h"

#include <iostream>
#include <vector>
#include "core/engine.h"
#include "utils/gl_utils.h"

#include "lab_m1/Tema2/transform3D.h"

using namespace std;
using namespace m1;


Tema2::Tema2()
{
	bodyDepth = 0.5f; // grosimea dronei
	bodyWidth = 1.0f; // lungimea dronei
	bodyHeight = 0.2f; // inaltimea dronei
	dronePosition = glm::vec3(0, 1, 0); //pozitia dronei
	droneRotation2 = glm::vec3(0, 0, 0); //rotatia dronei
	propellerRotation = 0.0f; //unghiul elicei
    armLength2 = bodyWidth * 1.2f;  // lungimea bratului

	CameraRotation = glm::vec3(0);

    culoareCorpMic = glm::vec3(1.0f, 0.0f, 1.0f);
	
    index = 0;  // index checkpoint (cand urmaeza urmatorul checkpoint)
	nr_checkpoints = 5;  // numarul de CHECKPOINTURI
	CheckPoints = std::vector<bool>(nr_checkpoints + 1, false);  // vector de checkpoint-uri

    // CAMERA E SETATA DOAR PENTRU FIRST PERSON, DACA SE ROTESTE IN THIRD NU O SA FIE AXATA PE DRONA
    perspectiva_drona = false; // daca e true arata drona (taste 1 si 2)
    val_z_perspectiva = 0; // O SCHIMBI CA SA POTI ARATA DRONA
	time_secunde = 90.0f; // timpul de joc care ne ajuta sa SCALAM BARA DE TIMP
}


Tema2::~Tema2()
{
}


void Tema2::Init()
{
    polygonMode = GL_FILL;
    glEnable(GL_DEPTH_TEST);
    // rezolutia pentru Mini viewport
    glm::ivec2 resolution = window->GetResolution();
    // pozite mini-view area si dimensiunea lui
    miniViewportArea = ViewportArea(50, 50, resolution.x / 2.f, resolution.y / 2.f);


    camera = new implemented::CameraLab();
    camera->Set(glm::vec3(dronePosition.x, dronePosition.y + 1, dronePosition.z), glm::vec3(dronePosition.x, dronePosition.y + 1, dronePosition.z), glm::vec3(0, 0, 0));
	camera->RotateFirstPerson_OX(RADIANS(droneRotation2.x));
	camera->RotateFirstPerson_OY(RADIANS(droneRotation2.y));
	camera->RotateFirstPerson_OZ(RADIANS(droneRotation2.z));
    //CAMERA O SA FIE DIN PERSPECTIVA THIRD PERSON
	// initiem pozitia camerei si rotatia pentru inceput
	GetSceneCamera()->SetPosition(glm::vec3(dronePosition.x , dronePosition.y + 1, dronePosition.z));
	GetSceneCamera()->SetRotation(CameraRotation);


    // SHADER TEREN
    {
        Shader* shader = new Shader("tema2");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    {
        Shader* shader = new Shader("umbra");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shader_traseu", "VertexShaderTraseu.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shader_traseu", "FragmentShaderTraseu.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    //TEREN
	CreateTeren();

    //COPACI
	CreateTree("brad");
    GenerateTrees(35);

	// CLADIRI
	CreateBuilding("cladire");
	GenerateBuildings(5);

    //DRONA
	CreateDrone();

	// CHECKPOINT
	CreateCheckPoint();
	GenerateCheckPoint(nr_checkpoints);

    // SAGEATA
    CreateArrowMesh();

    //TIMER
	CreateTimer("timer", 0.1f, 0.1f, glm::vec3(1.0f, 0.0f, 0.0f));
    CreateTimerContur("timercontur", 0.1f, 0.1f, glm::vec3(1.0f, 1.0f, 1.0f));
}

// --------------------  CREARE TEREN  ------------------------------
void Tema2::CreateTeren() {
	// Dimensiunea grilei
	int dimensiune_grid = 100;
	float pasi = 10.0f;
	glm::vec3 culoare_teren = glm::vec3(0.5f, 0.5f, 0.5f); // gri

    // Calcularea offset-ului pentru centrare in origine
    float offset = (dimensiune_grid - 1) * pasi / 2.0f;

    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Generarea vertecsilor pentru grila
    for (int i = 0; i < dimensiune_grid; ++i) {
        for (int j = 0; j < dimensiune_grid; ++j) {
            // aplic offsetul pentru a centra terenul in (0,0,0)
            glm::vec3 position(i * pasi - offset, 0.0f, j * pasi - offset);
            vertices.push_back(VertexFormat(position, culoare_teren));
        }
    }

    // Generarea indecsilor pentru grila
    for (int i = 0; i < dimensiune_grid - 1; ++i) {
        for (int j = 0; j < dimensiune_grid - 1; ++j) {
            int topLeft = i * dimensiune_grid + j;
            int topRight = topLeft + 1;
            int bottomLeft = (i + 1) * dimensiune_grid + j;
            int bottomRight = bottomLeft + 1;

            // Doua triunghiuri pentru fiecare patrat
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    CreateMesh("teren", vertices, indices);
}

//-----------------------------  CREARE COPACI  --------------------------------
void Tema2::CreateTree(const std::string& name) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Trunchiul (cilindru)
    float heightTrunk = 5.0f; // inaltimea trunchiului
    float radiusTrunk = 1.0f; // raza trunchiului
    int slices = 100; // numarul de fete ale cilindrului
    glm::vec3 culoare_trunchi = glm::vec3(0.55f, 0.27f, 0.07f); // culoare trunchi

    for (int i = 0; i <= slices; i++) {
        float angle = i * glm::two_pi<float>() / slices;  // impartim 360 de grade in parti
        float x = radiusTrunk * cos(angle);
        float z = radiusTrunk * sin(angle);

        glm::vec3 normal = glm::normalize(glm::vec3(x, 0, z));
        vertices.emplace_back(glm::vec3(x, 0, z), normal, culoare_trunchi); // baza cilindrului
        vertices.emplace_back(glm::vec3(x, heightTrunk, z), normal, culoare_trunchi); // varful cilindrului
    }

    for (int i = 0; i < slices; i++) {
        indices.push_back(i * 2);
        indices.push_back(i * 2 + 1);
        indices.push_back(i * 2 + 2);

        indices.push_back(i * 2 + 1);
        indices.push_back(i * 2 + 2);
        indices.push_back(i * 2 + 3);
    }

    // Coroana (doua conuri)
    float heightCrown = 4.0f;
    float radiusCrown = 3.0f;
    glm::vec3 culoare_coroana_sus = glm::vec3(0.0f, 0.5f, 0.0f); // verde actual
    glm::vec3 culoare_coroana_jos = glm::vec3(0.0f, 0.3f, 0.0f); // verde inchis
    float pozitie_start_coroana = heightTrunk - 1.5f;

    // Primul con
    for (int i = 0; i <= slices; ++i) {
        float angle = i * glm::two_pi<float>() / slices;
        float x = radiusCrown * cos(angle);
        float z = radiusCrown * sin(angle);

        // Calculam gradientul de culoare
        float t = (pozitie_start_coroana - heightTrunk) / heightCrown;
        glm::vec3 culoare_coroana = glm::mix(culoare_coroana_jos, culoare_coroana_sus, t);

        vertices.emplace_back(glm::vec3(x, pozitie_start_coroana, z), glm::normalize(glm::vec3(x, heightCrown, z)), culoare_coroana);
    }
    vertices.emplace_back(glm::vec3(0, pozitie_start_coroana + heightCrown, 0), glm::vec3(0, 1, 0), culoare_coroana_sus);

    for (int i = 0; i < slices; ++i) {
        indices.push_back(vertices.size() - 1);
        indices.push_back(vertices.size() - 2 - i);
        indices.push_back(vertices.size() - 3 - i);
    }

    // Al doilea con
    pozitie_start_coroana += heightCrown / 2; // Mutam al doilea con mai sus deasupra primului
    radiusCrown *= 0.75f; // Reduce raza pentru al doilea con

    for (int i = 0; i <= slices; ++i) {
        float angle = i * glm::two_pi<float>() / slices;
        float x = radiusCrown * cos(angle);
        float z = radiusCrown * sin(angle);

        // Calculam gradientul de culoare
        float t = (pozitie_start_coroana - heightTrunk) / heightCrown;
        glm::vec3 culoare_coroana = glm::mix(culoare_coroana_jos, culoare_coroana_sus, t);

        vertices.emplace_back(glm::vec3(x, pozitie_start_coroana, z), glm::normalize(glm::vec3(x, heightCrown, z)), culoare_coroana);
    }
    vertices.emplace_back(glm::vec3(0, pozitie_start_coroana + heightCrown, 0), glm::vec3(0, 1, 0), culoare_coroana_sus);

    for (int i = 0; i < slices; ++i) {
        indices.push_back(vertices.size() - 1);
        indices.push_back(vertices.size() - 2 - i);
        indices.push_back(vertices.size() - 3 - i);
    }

    CreateMesh(name.c_str(), vertices, indices);
}

// ----- GENERARE COPACI POZITII -------
void Tema2::GenerateTrees(int count) {
	srand(static_cast<unsigned>(time(0))); // GENERARE ALEATORIE
    float gridSize = 200.0f; // Dimensiunea terenului unde punem copaci, daca vrem sa marim numarul de copaci marim si dimensiunea terenului altfel nu se incarca jocul
    float treeSpacing = 20.0f; // Minim distanta intre copaci
	float offset = gridSize / 2.0f; // Offset pentru a centra copacii

    for (int i = 0; i < count; ++i) {
        glm::vec3 position;
        bool validPosition;

        do {
            validPosition = true;
            position = glm::vec3(rand() % static_cast<int>(gridSize) - offset, 0, rand() % static_cast<int>(gridSize) - offset);

            for (const auto& tree : treePositions) {
                if (glm::distance(position, tree) < treeSpacing) {
                    validPosition = false;
                    break;
                }
            }

            // valid position sa nu fie in (0,0,0) la o distanta mai mica de 4
            if (glm::distance(position, glm::vec3(0, 0, 0)) < 4.0f) {
                validPosition = false;
            }

        } while (!validPosition);

        treePositions.push_back(position);
    }
}

//----------------------- CREARE CLADIRI ----------------------------

void Tema2::CreateBuilding(const std::string& name) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Dimensiunile clădirii
	float width = 15.0f; // lungime
	float height = 7.0f; // inaltimea
	float depth = 8.0f; // adancimea/ latime
    glm::vec3 culoare_cladire_exterior = glm::vec3(0.2f, 0.2f, 0.2f); // gri inchis
    glm::vec3 culoare_cladire_interior = glm::vec3(0.7f, 0.7f, 0.7f); // gri deschis

    // Vertices
    glm::vec3 normal_front_back = glm::vec3(0, 0, 1);
    glm::vec3 normal_left_right = glm::vec3(1, 0, 0);
    glm::vec3 normal_top_bottom = glm::vec3(0, 1, 0);

    // Fata frontala si spate
    for (int i = 0; i <= 1; ++i) {
        vertices.emplace_back(glm::vec3(-width / 2, 0, -depth / 2 + i * depth), normal_front_back, culoare_cladire_exterior);
        vertices.emplace_back(glm::vec3(width / 2, 0, -depth / 2 + i * depth), normal_front_back, culoare_cladire_exterior);
        vertices.emplace_back(glm::vec3(width / 2, height, -depth / 2 + i * depth), normal_front_back, culoare_cladire_exterior);
        vertices.emplace_back(glm::vec3(-width / 2, height, -depth / 2 + i * depth), normal_front_back, culoare_cladire_exterior);
    }

    // Fata stanga si dreapta
    for (int i = 0; i <= 1; ++i) {
        vertices.emplace_back(glm::vec3(-width / 2 + i * width, 0, -depth / 2), normal_left_right, culoare_cladire_exterior);
        vertices.emplace_back(glm::vec3(-width / 2 + i * width, 0, depth / 2), normal_left_right, culoare_cladire_exterior);
        vertices.emplace_back(glm::vec3(-width / 2 + i * width, height, depth / 2), normal_left_right, culoare_cladire_exterior);
        vertices.emplace_back(glm::vec3(-width / 2 + i * width, height, -depth / 2), normal_left_right, culoare_cladire_exterior);
    }

    // Fata de sus si jos
    for (int i = 0; i <= 1; ++i) {
        vertices.emplace_back(glm::vec3(-width / 2, i * height, -depth / 2), normal_top_bottom, culoare_cladire_interior);
        vertices.emplace_back(glm::vec3(width / 2, i * height, -depth / 2), normal_top_bottom, culoare_cladire_interior);
        vertices.emplace_back(glm::vec3(width / 2, i * height, depth / 2), normal_top_bottom, culoare_cladire_interior);
        vertices.emplace_back(glm::vec3(-width / 2, i * height, depth / 2), normal_top_bottom, culoare_cladire_interior);
    }

    // indesci
    for (int i = 0; i < 6; ++i) {
        int offset = i * 4;

        indices.push_back(offset);
        indices.push_back(offset + 1);
        indices.push_back(offset + 2);
        indices.push_back(offset);
        indices.push_back(offset + 2);
        indices.push_back(offset + 3);
    }

    CreateMesh(name.c_str(), vertices, indices);
}

//------------------- GENERARE CLADIRI POZITII -------------------------

void Tema2::GenerateBuildings(int count) {
    srand(static_cast<unsigned>(time(0))); // GENERARE ALEATORIE
    float gridSize = 200.0f; // Dimensiunea terenului
    float buildingSpacing = 30.0f; // Minim distanta intre cladiri si copaci
    float offset = gridSize / 2.0f; // Offset pentru a centra cladirile

    for (int i = 0; i < count; ++i) {
        glm::vec3 position;
        bool validPosition;

        do {
            validPosition = true;
            position = glm::vec3(rand() % static_cast<int>(gridSize) - offset, 0, rand() % static_cast<int>(gridSize) - offset);

			// sa nu se puna cladirile peste copaci
            for (const auto& tree : treePositions) {
                if (glm::distance(position, tree) < buildingSpacing) {
                    validPosition = false;
                    break;
                }
            }

			// sa nu se puna cladirile peste alte cladiri
            for (const auto& building : buildingPositions) {
                if (glm::distance(position, building) < buildingSpacing) {
                    validPosition = false;
                    break;
                }
            }

            // valid position sa nu fie in (0,0,0) la o distanta mai mica de 3
			if (glm::distance(position, glm::vec3(0, 0, 0)) < 10.0f) {
				validPosition = false;
			}

        } while (!validPosition);

        buildingPositions.push_back(position);
    }
}


// --------------------- CREARE DRONA ----------------------------
void Tema2::CreateDrone()
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    glm::vec3 culoareCorp = glm::vec3(0.7f, 0.7f, 0.8f); // albastru-gri
    glm::vec3 culoareElice = glm::vec3(0.0f, 0.0f, 0.0f); // negru

    // Vertices si indici pentru paralelipipedul în forma de X
	float armLength = bodyWidth * 1.3f; // lungimea bratului ( cea global e mai mic cu dimensiunea cubului = 0.1f fata de asta)
	float armThickness = 0.1f; // grosimea bratului

    // pozitie, normala, culoare, normala e pentru a pune culoarea deoarece folosim shader-ul Normal
    vertices = {
        // Brat orizontal
        VertexFormat(glm::vec3(-armLength, 0, -armThickness), glm::vec3(0, 1, 0), culoareCorp),
        VertexFormat(glm::vec3(armLength, 0, -armThickness), glm::vec3(0, 1, 0), culoareCorp),
        VertexFormat(glm::vec3(armLength, bodyHeight, -armThickness), glm::vec3(0, 1, 0), culoareCorp),
        VertexFormat(glm::vec3(-armLength, bodyHeight, -armThickness), glm::vec3(0, 1, 0), culoareCorp),

        VertexFormat(glm::vec3(-armLength, 0, armThickness), glm::vec3(0, 1, 0), culoareCorp),
        VertexFormat(glm::vec3(armLength, 0, armThickness), glm::vec3(0, 1, 0), culoareCorp),
        VertexFormat(glm::vec3(armLength, bodyHeight, armThickness), glm::vec3(0, 1, 0), culoareCorp),
        VertexFormat(glm::vec3(-armLength, bodyHeight, armThickness), glm::vec3(0, 1, 0), culoareCorp),

        // Brat vertical
        VertexFormat(glm::vec3(-armThickness, 0, -armLength), glm::vec3(1, 0, 0), culoareCorp),
        VertexFormat(glm::vec3(armThickness, 0, -armLength), glm::vec3(1, 0, 0), culoareCorp),
        VertexFormat(glm::vec3(armThickness, bodyHeight, -armLength), glm::vec3(1, 0, 0), culoareCorp),
        VertexFormat(glm::vec3(-armThickness, bodyHeight, -armLength), glm::vec3(1, 0, 0), culoareCorp),

        VertexFormat(glm::vec3(-armThickness, 0, armLength), glm::vec3(1, 0, 0), culoareCorp),
        VertexFormat(glm::vec3(armThickness, 0, armLength), glm::vec3(1, 0, 0), culoareCorp),
        VertexFormat(glm::vec3(armThickness, bodyHeight, armLength), glm::vec3(1, 0, 0), culoareCorp),
        VertexFormat(glm::vec3(-armThickness, bodyHeight, armLength), glm::vec3(1, 0, 0), culoareCorp)
    };

    indices = {
        // Brat orizontal - fete laterale
        0, 1, 2, 0, 2, 3, // Fata spate
        4, 5, 6, 4, 6, 7, // Fata fata
        0, 3, 7, 0, 7, 4, // Margine stanga
        1, 2, 6, 1, 6, 5, // Margine dreapta

        // Fetele de sus si jos ale bratului orizontal
        3, 2, 6, 3, 6, 7, // Sus
        0, 1, 5, 0, 5, 4, // Jos

        // Brar vertical - fete laterale
        8, 9, 10, 8, 10, 11, // Fata spate
        12, 13, 14, 12, 14, 15, // Fata fata
        8, 11, 15, 8, 15, 12, // Margine stanga
        9, 10, 14, 9, 14, 13, // Margine dreapta

        // Fetele de sus si jos ale bratului vertical
        11, 10, 14, 11, 14, 15, // Sus
        8, 9, 13, 8, 13, 12  // Jos
    };

    CreateMesh("droneBody", vertices, indices);

    // Creeaza cuburi suport pentru elice
    float cubeSize = 0.1f; // = grosimea dronei
    CreateCube("supportCube", cubeSize, culoareCorp);

    // Creeaza elicile (dreptunghiuri subtiri)
    float propellerWidth = 0.5f;
    float propellerHeight = 0.051f;
    CreateElice("propeller", propellerWidth, propellerHeight, culoareElice);
}


void Tema2::CreateCube(const std::string& name, float size, glm::vec3 color) {
    std::vector<VertexFormat> vertices = {
        VertexFormat(glm::vec3(-size, -size, size), glm::vec3(-1, -1, 1), color),
        VertexFormat(glm::vec3(size, -size, size), glm::vec3(1, -1, 1), color),
        VertexFormat(glm::vec3(size, size, size), glm::vec3(1, 1, 1), color),
        VertexFormat(glm::vec3(-size, size, size), glm::vec3(-1, 1, 1), color),
        VertexFormat(glm::vec3(-size, -size, -size), glm::vec3(-1, -1, -1), color),
        VertexFormat(glm::vec3(size, -size, -size), glm::vec3(1, -1, -1), color),
        VertexFormat(glm::vec3(size, size, -size), glm::vec3(1, 1, -1), color),
        VertexFormat(glm::vec3(-size, size, -size), glm::vec3(-1, 1, -1), color)
    };

    std::vector<unsigned int> indices = {
        0, 1, 2, 0, 2, 3, 1, 5, 6, 1, 6, 2, 5, 4, 7, 5, 7, 6, 4, 0, 3, 4, 3, 7, 3, 2, 6, 3, 6, 7, 1, 0, 4, 1, 4, 5
    };

    CreateMesh(name.c_str(), vertices, indices);
}

void Tema2::CreateElice(const std::string& name, float width, float height, glm::vec3 color) {
    std::vector<VertexFormat> vertices = {
        VertexFormat(glm::vec3(-width, 0, -height), glm::vec3(-1, 0, -1), color),
        VertexFormat(glm::vec3(width, 0, -height), glm::vec3(1, 0, -1), color),
        VertexFormat(glm::vec3(width, 0, height),  glm::vec3(1, 0, 1),color),
        VertexFormat(glm::vec3(-width, 0, height), glm::vec3(-1, 0, 1), color)
    };

    std::vector<unsigned int> indices = { 0, 1, 2, 0, 2, 3 };
    CreateMesh(name.c_str(), vertices, indices);
}

//-------------------------------- CHECK-POINT ---------------------------------
void Tema2::CreateCheckPoint() {

    float lungime = 3.3f; // lungimea batului
    float grosime = 0.1f; // grosimea batului
	float distanta = 1.5f; // distanta dintre centru si picior
    glm::vec3 culoareCorp = glm::vec3(1.0f, 1.0f, 1.0f); // alb
    CreateParaVerti("piciorstanga", -distanta, lungime, grosime, culoareCorp, 0.0f); // 0 = deunde incepe sa deseneze pe axa y
    CreateParaVerti("piciordreapta", distanta, lungime, grosime, culoareCorp, 0.0f);
    // chenar mare
    float distantaChenarM = distanta + 1.0f;
    float inaltime_chenar = 3.0f;
    float inaltime_offset = lungime + 3.0f;
	CreateParaOrizont("parte_jos_M", distantaChenarM, grosime, culoareCorp, lungime);
    CreateParaOrizont("parte_sus_M", distantaChenarM, grosime, culoareCorp, inaltime_offset);
    CreateParaVerti("parte_stanga_M", -distantaChenarM, inaltime_chenar, grosime, culoareCorp, lungime);
    CreateParaVerti("parte_dreapta_M", distantaChenarM - grosime, inaltime_chenar, grosime, culoareCorp, lungime);

    float offset_patrat_mic = 0.3f; // offset pentru patratul mic
	float distanta_patrat_mic = distantaChenarM - offset_patrat_mic;
    float inaltime1_mic = lungime + offset_patrat_mic;
	float inaltime2_mic = inaltime_offset - offset_patrat_mic;
    CreateParaOrizont("parte_jos_mic", distanta_patrat_mic, grosime, culoareCorpMic, inaltime1_mic);
    CreateParaOrizont("parte_sus_mic", distanta_patrat_mic, grosime, culoareCorpMic, inaltime2_mic);
    CreateParaVerti("parte_stanga_mic", -distanta_patrat_mic, inaltime_chenar - (2 * offset_patrat_mic), grosime, culoareCorpMic, inaltime1_mic);
    CreateParaVerti("parte_dreapta_mic", distanta_patrat_mic - grosime, inaltime_chenar - (2 * offset_patrat_mic), grosime, culoareCorpMic, inaltime1_mic);

}

void Tema2::CreateParaOrizont(const std::string& name, float distantaCenter, float grosime, glm::vec3 culoareCorp, float inaltime) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    vertices = {
        VertexFormat(glm::vec3(-distantaCenter, inaltime, -grosime), glm::vec3(0, 1, 0), culoareCorp), // jos stanga
        VertexFormat(glm::vec3(distantaCenter, inaltime, -grosime), glm::vec3(0, 1, 0), culoareCorp), // jos dreapta
        VertexFormat(glm::vec3(distantaCenter, inaltime, grosime), glm::vec3(0, 1, 0), culoareCorp), //  sus dreapta
        VertexFormat(glm::vec3(-distantaCenter, inaltime, grosime), glm::vec3(0, 1, 0), culoareCorp), //  sus stanga

        // Partea din spate
        VertexFormat(glm::vec3(-distantaCenter, inaltime + grosime, -grosime), glm::vec3(0, 1, 0), culoareCorp), // jos stanga
        VertexFormat(glm::vec3(distantaCenter , inaltime + grosime, -grosime), glm::vec3(0, 1, 0), culoareCorp), // jos dreapta
        VertexFormat(glm::vec3(distantaCenter , inaltime + grosime, grosime), glm::vec3(0, 1, 0), culoareCorp), // sus dreapta
        VertexFormat(glm::vec3(-distantaCenter, inaltime + grosime, grosime), glm::vec3(0, 1, 0), culoareCorp), //  sus stanga
    };

    indices = {
        // Fete laterale
       0, 1, 2, 0, 2, 3, // Fata din spate
       4, 5, 6, 4, 6, 7, // Fata din fata
       0, 3, 7, 0, 7, 4, // Margine stanga
       1, 2, 6, 1, 6, 5, // Margine dreapta

       // Fetele de sus si jos 
       3, 2, 6, 3, 6, 7, // Sus
       0, 1, 5, 0, 5, 4, // Jos
    };

    CreateMesh(name.c_str(), vertices, indices);
}

void Tema2::CreateParaVerti(const std::string& name, float distantaCenter, float lungime, float grosime, glm::vec3 culoareCorp, float start_inaltime ) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    vertices = {
       VertexFormat(glm::vec3(distantaCenter, start_inaltime , -grosime), glm::vec3(1, 0, 0), culoareCorp), // jos stanga
       VertexFormat(glm::vec3(distantaCenter, start_inaltime + lungime, -grosime), glm::vec3(1, 0, 0), culoareCorp),  // sus stanga
       VertexFormat(glm::vec3(distantaCenter + grosime, start_inaltime + lungime, -grosime), glm::vec3(1, 0, 0), culoareCorp), //  sus dreapta
       VertexFormat(glm::vec3(distantaCenter + grosime, start_inaltime, -grosime), glm::vec3(1, 0, 0), culoareCorp), //jos dreapta

       VertexFormat(glm::vec3(distantaCenter, start_inaltime, grosime), glm::vec3(1, 0, 0), culoareCorp),  // jos stanga (fata din fata)
       VertexFormat(glm::vec3(distantaCenter, start_inaltime + lungime, grosime), glm::vec3(1, 0, 0), culoareCorp),   // sus stanga (fata din fata)
       VertexFormat(glm::vec3(distantaCenter + grosime, start_inaltime + lungime, grosime), glm::vec3(1, 0, 0), culoareCorp), // sus dreapta (fata din fata)
       VertexFormat(glm::vec3(distantaCenter + grosime, start_inaltime, grosime), glm::vec3(1, 0, 0), culoareCorp), // jos dreapta (fata din fata)
    };


    indices = {
        //fete laterale 
        0, 1, 2, 0, 2, 3, // Fata din spate
        4, 5, 6, 4, 6, 7, // Fata din fata
        0, 3, 7, 0, 7, 4, // Margine stanga
        1, 2, 6, 1, 6, 5, // Margine dreapta

        // Fetele de sus si jos
        3, 2, 6, 3, 6, 7, // Sus
        0, 1, 5, 0, 5, 4, // Jos
    };

    CreateMesh(name.c_str(), vertices, indices);
}


void Tema2::GenerateCheckPoint(int count) {
	srand(static_cast<unsigned>(time(0))); // GENERARE ALEATORIE
    float gridSize = 200.0f; // Dimensiunea terenului unde punem obstacole
    float obstacleSpacing = 20.0f; // Minim distanta intre obstacole
    float offset = gridSize / 2.0f; // Offset pentru a centra obstacolele

    for (int i = 0; i < count; ++i) {
        glm::vec3 position;
        bool validPosition;

        do {
            validPosition = true;
            position = glm::vec3(rand() % static_cast<int>(gridSize) - offset, 0, rand() % static_cast<int>(gridSize) - offset);
            
			// nu pune obstacolele peste copaci sau cladiri
            for (const auto& tree : treePositions) {
                if (glm::distance(position, tree) < obstacleSpacing) {
                    validPosition = false;
                    break;
                }
            }

            for (const auto& building : buildingPositions) {
                if (glm::distance(position, building) < obstacleSpacing) {
                    validPosition = false;
                    break;
                }
            }

			// nu pune obstacolele peste alte obstacole
            for (const auto& obstacle : check_point_position) {
                if (glm::distance(position, obstacle) < obstacleSpacing) {
                    validPosition = false;
                    break;
                }
            }

			// valid position sa nu fie in (0,0,0) la o distanta mai mica de 2
			if (glm::distance(position, glm::vec3(0, 0, 0)) < 2.0f) {
				validPosition = false;
			}

        } while (!validPosition);

        check_point_position.push_back(position);
    }
}


void Tema2::CreateArrowMesh() {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Corpul sagetii (un dreptunghi)
	vertices.push_back(VertexFormat(glm::vec3(-0.5f, 0.0f, -2.0f), glm::vec3(1, 0, 0), glm::vec3(0.2f, 0.5f, 0.0f))); // jos stanga
	vertices.push_back(VertexFormat(glm::vec3(0.5f, 0.0f, -2.0f), glm::vec3(1, 0, 0), glm::vec3(0.2f, 0.5f, 0.0f))); // jos dreapta
	vertices.push_back(VertexFormat(glm::vec3(0.5f, 0.0f, 2.0f), glm::vec3(1, 0, 0), glm::vec3(0.2f, 0.5f, 0.0f))); // sus dreapta
	vertices.push_back(VertexFormat(glm::vec3(-0.5f, 0.0f, 2.0f), glm::vec3(1, 0, 0), glm::vec3(0.2f, 0.5f, 0.0f))); // sus stanga

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(2);
    indices.push_back(3);
    indices.push_back(0);


    // Capul sagetii (un triunghi)
    vertices.push_back(VertexFormat(glm::vec3(-1.0f, 0.0f, 2.0f), glm::vec3(1, 0, 0), glm::vec3(0.2f, 0.5f, 0.0f)));
    vertices.push_back(VertexFormat(glm::vec3(1.0f, 0.0f, 2.0f), glm::vec3(1, 0, 0), glm::vec3(0.2f, 0.5f, 0.0f)));
    vertices.push_back(VertexFormat(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(1, 0, 0), glm::vec3(0.2f, 0.5f, 0.0f)));

    indices.push_back(4);
    indices.push_back(5);
    indices.push_back(6);

    CreateMesh("arrow", vertices, indices);
}

void Tema2::RenderArrow(glm::vec3 position, glm::vec3 direction) {
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::rotate(modelMatrix, atan2(direction.x, direction.z), glm::vec3(0, 1, 0));
    RenderSimpleMesh(meshes["arrow"], shaders["tema2"], modelMatrix, 0.0f, glm::vec3(0.6f, 0.2f, 0.6f));
}


void Tema2::CreateTimer(const std::string& name, float width, float height, glm::vec3 color) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Definim vertecsii dreptunghiului pe planul XOY
	vertices.emplace_back(glm::vec3(-width/2, -height / 2, 0), glm::vec3(0, 0, 1), color); // jos stanga
	vertices.emplace_back(glm::vec3(0, -height / 2, 0), glm::vec3(0, 0, 1), color); // jos dreapta
	vertices.emplace_back(glm::vec3(0, height / 2, 0), glm::vec3(0, 0, 1), color); // sus dreapta
	vertices.emplace_back(glm::vec3(-width/2, height / 2, 0), glm::vec3(0, 0, 1), color); // sus stanga

    // Definim indecsii pentru cele doua triunghiuri care formeaza dreptunghiul
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);


    CreateMesh(name.c_str(), vertices, indices);
}

void Tema2::CreateTimerContur(const std::string& name, float width, float height, glm::vec3 color) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Definim vertecaii dreptunghiului pe planul XOY
    vertices.emplace_back(glm::vec3(-width / 2, -height / 2, 0), glm::vec3(0, 0, 1), color); // jos stanga
    vertices.emplace_back(glm::vec3(0, -height / 2, 0), glm::vec3(0, 0, 1), color); // jos dreapta
    vertices.emplace_back(glm::vec3(0, height / 2, 0), glm::vec3(0, 0, 1), color); // sus dreapta
    vertices.emplace_back(glm::vec3(-width / 2, height / 2, 0), glm::vec3(0, 0, 1), color); // sus stanga

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);;
    indices.push_back(3);


	CreateMeshLine(name.c_str(), vertices, indices);  // MESH CARE FACE DOAR CONTURUL
}

void Tema2::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(137.0f / 255.0f, 207.0f / 255.0f, 250.0f / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void Tema2::RenderScene(float deltaTimeSeconds) {

	// RANDARE TEREN
    {
        modelMatrix = glm::mat4(1);
        RenderSimpleMesh(meshes["teren"], shaders["tema2"], modelMatrix, 0.1f, glm::vec3(0.0f, 0.5f, 0.5f));
    }

    // RANDARE COPACI
    {
        for (const auto& position : treePositions) {
            modelMatrix = glm::mat4(1);
            modelMatrix *= transform3D::Translate(position.x, position.y + 0.7, position.z); // am adaugat 0.7 din cauza noise-ului de la teren pentru a se vedea bine umbra
            RenderMesh(meshes["brad"], shaders["VertexNormal"], modelMatrix);
            RenderUmbraMesh(meshes["brad"], shaders["umbra"], modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f), glm::normalize(glm::vec3(1.0f, -1.0f, 1.0f)));  //umbra
        }
    }

    // RANDARE CLĂDIRI
    {
        for (const auto& position : buildingPositions) {
            modelMatrix = glm::mat4(1);
            modelMatrix *= transform3D::Translate(position.x, position.y + 0.7, position.z); // am adaugat 0.7 din cauza noise-ului de la teren pentru a se vedea bine umbra
			RenderMesh(meshes["cladire"], shaders["VertexNormal"], modelMatrix);
            RenderUmbraMesh(meshes["cladire"], shaders["umbra"], modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f), glm::normalize(glm::vec3(1.0f, -1.0f, 1.0f))); //umbra, ultima valoare este directia luminii
        }
    }

	// RANDARE CHECK-POINT FARA PATRATUL MIC
	{
        for (int i = 0; i < check_point_position.size(); i++) {

            modelMatrix = glm::mat4(1);
            modelMatrix *= transform3D::Translate(check_point_position[i].x, check_point_position[i].y, check_point_position[i].z);
            // PARTEA EXTERIOARA A CHECK POINTULUI
            RenderSimpleMesh(meshes["obstacol"], shaders["tema2"], modelMatrix, 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
            RenderSimpleMesh(meshes["piciorstanga"], shaders["tema2"], modelMatrix, 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
            RenderSimpleMesh(meshes["piciordreapta"], shaders["tema2"], modelMatrix, 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
            RenderSimpleMesh(meshes["parte_jos_M"], shaders["tema2"], modelMatrix, 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
            RenderSimpleMesh(meshes["parte_sus_M"], shaders["tema2"], modelMatrix, 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
            RenderSimpleMesh(meshes["parte_stanga_M"], shaders["tema2"], modelMatrix, 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
            RenderSimpleMesh(meshes["parte_dreapta_M"], shaders["tema2"], modelMatrix, 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));


		}
	}

    //RANDARE PATRAT MIC + SCHIMBARE CULORI
    for (int i = 0; i < check_point_position.size(); i++) {

        // Distanta dintre drona si checkpoint
        float dist = glm::distance(check_point_position[i], dronePosition);

        // Stabilim culoarea în functie de starea checkpoint-ului
        if (i < index) {
            culoareCorpMic = glm::vec3(0.0f, 0.5f, 0.2f); // VERDE pentru checkpoint-uri VIZITAT
        }
        else if (i == index) {

            culoareCorpMic = glm::vec3(1.0f, 0.0f, 0.0f); // ROSU pentru checkpoint-ul ACTIV prin care trebui SA TRECEM,

            // Verificam daca drona a trecut prin checkpoint-ul curent
            if (dronePosition.z > check_point_position[i].z) {
				culoareCorpMic = glm::vec3(1.0f, 0.5f, 0.0f); // PORTOCALIU pentru atunci cand ne AFLAM IN FATA checkpoint-ului pe PARTEA BUNA
            }
            if (dist < 5.0f) {
                // in momentul in care se afla in interiorul chenarului mic si trece de z check-poitului vom marca 
                if (check_point_position[i].x + 2.5f > dronePosition.x && check_point_position[i].x - 2.5f < dronePosition.x && check_point_position[i].y + 3.6f < dronePosition.y && check_point_position[i].y + 6.0f > dronePosition.y && dronePosition.z < check_point_position[i].z) {
                    CheckPoints[i] = true; // Marcare ca vizitat
                    index++;  // Trecem la următorul checkpoint
                }
            }
        }
        else {
            culoareCorpMic = glm::vec3(1.0f, 1.0f, 1.0f); // ALB pentru checkpoint-uri NEACTIVE
        }

        modelMatrix = glm::translate(glm::mat4(1), check_point_position[i]);
        RenderSimpleMesh(meshes["parte_jos_mic"], shaders["tema2"], modelMatrix, 0.0f, culoareCorpMic);
        RenderSimpleMesh(meshes["parte_sus_mic"], shaders["tema2"], modelMatrix, 0.0f, culoareCorpMic);
        RenderSimpleMesh(meshes["parte_stanga_mic"], shaders["tema2"], modelMatrix, 0.0f, culoareCorpMic);
        RenderSimpleMesh(meshes["parte_dreapta_mic"], shaders["tema2"], modelMatrix, 0.0f, culoareCorpMic);
    }


    //RANDARE DRONA 

    // MISCARE DRONA 
    glm::quat droneQuaternion = glm::quat(glm::radians(glm::vec3(
        droneRotation2.y,  
        droneRotation2.x, 
        droneRotation2.z  
    )));

    camera->Set(glm::vec3(dronePosition.x, dronePosition.y + 1, dronePosition.z), glm::vec3(dronePosition.x, dronePosition.y + 1, dronePosition.z), glm::vec3(0, 0, 0));
    camera->RotateFirstPerson_OX(RADIANS(droneRotation2.x));
    camera->RotateFirstPerson_OY(RADIANS(droneRotation2.y));
    camera->RotateFirstPerson_OZ(RADIANS(droneRotation2.z));

    GetSceneCamera()->SetPosition(glm::vec3(dronePosition.x, dronePosition.y + 1, dronePosition.z + val_z_perspectiva));
	GetSceneCamera()->SetRotation(droneQuaternion);

	// ROTATIA ELICELOR
    propellerRotation += deltaTimeSeconds * 200; // Rotatie continua

    // RANDARE DRONA
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, dronePosition);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(droneRotation2.x), glm::vec3(0, 1, 0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(droneRotation2.y), glm::vec3(1, 0, 0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(droneRotation2.z), glm::vec3(0, 0, 1));
    RenderMesh(meshes["droneBody"], shaders["VertexNormal"], modelMatrix);

    // pozitiile cuburilor de suport
    glm::vec3 propellerOffsets[4] = {
        glm::vec3(-armLength2, bodyHeight, 0),
        glm::vec3(armLength2, bodyHeight, 0),
        glm::vec3(0, bodyHeight, -armLength2),
        glm::vec3(0, bodyHeight, armLength2)
    };


    for (int i = 0; i < 4; i++) {
        glm::vec3 offset = propellerOffsets[i];

        // Cub suport
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, dronePosition);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(droneRotation2.x), glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(droneRotation2.y), glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(droneRotation2.z), glm::vec3(0, 0, 1));
		modelMatrix = glm::translate(modelMatrix, offset); // offsetul fata de centrul dronei pentru a pozitiona cuburile
        RenderMesh(meshes["supportCube"], shaders["VertexNormal"], modelMatrix);

        // Elice
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, dronePosition);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(droneRotation2.x), glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(droneRotation2.y), glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(droneRotation2.z), glm::vec3(0, 0, 1));
        modelMatrix = glm::translate(modelMatrix, offset + glm::vec3(0, 0.11f, 0)); // glm::vec3(0, 0.11f, 0) = inaltimea cubului de suport, 0,01 ca sa vina fix in cub
        modelMatrix = glm::rotate(modelMatrix, glm::radians(propellerRotation), glm::vec3(0, 1, 0));
        RenderMesh(meshes["propeller"], shaders["VertexNormal"], modelMatrix);
    }

}

// ---------------------------- MESH  ----------------------------
Mesh* Tema2::CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices)
{
    unsigned int VAO = 0;
    // Create the VAO and bind it
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create the VBO and bind it
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Send vertices data into the VBO buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    // Create the IBO and bind it
    unsigned int IBO;
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    // Send indices data into the IBO buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

    // ========================================================================

    // Set vertex position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), 0);

    //// Set vertex normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec3)));

    //// Set texture coordinate attribute
    //glEnableVertexAttribArray(2);
    //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3)));

    //// Set vertex color attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));

    // Unbind the VAO
    glBindVertexArray(0);

    // Check for OpenGL errors
    CheckOpenGLError();

    // Mesh information is saved into a Mesh object
    meshes[name] = new Mesh(name);
    meshes[name]->InitFromBuffer(VAO, static_cast<unsigned int>(indices.size()));
    meshes[name]->vertices = vertices;
    meshes[name]->indices = indices;
    return meshes[name];
}

// ------------------------ MESH CONTUR ------------------------
Mesh* Tema2::CreateMeshLine(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices)
{
    unsigned int VAO = 0;
    // Create the VAO and bind it
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create the VBO and bind it
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Send vertices data into the VBO buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    // Create the IBO and bind it
    unsigned int IBO;
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    // Send indices data into the IBO buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

    // ========================================================================

    // Set vertex position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), 0);

    //// Set vertex normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec3)));

    //// Set vertex color attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));

    // Unbind the VAO
    glBindVertexArray(0);

    // Check for OpenGL errors
    CheckOpenGLError();

    // Mesh information is saved into a Mesh object
    meshes[name] = new Mesh(name);
    meshes[name]->InitFromBuffer(VAO, static_cast<unsigned int>(indices.size()));
    meshes[name]->SetDrawMode(GL_LINE_LOOP);  //contur
    meshes[name]->vertices = vertices;
    meshes[name]->indices = indices;
    return meshes[name];
}

// --------- Se verifica daca s-au parcurs toate check-pointurile ---------
bool Tema2::VerificaCheckPoint() {
    for (int i = 0; i < nr_checkpoints; i++) {
        if (!CheckPoints[i]) {
            return false;
        }
    }
    return true;
}

void Tema2::Update(float deltaTimeSeconds)
{
    // pentru poligoane cand se apasa space sa se vada ori structura ori punctele vetecsii
    glLineWidth(3);
    glPointSize(5);
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

    // zona de desenare
    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);

	//--------------------------------------------------------------------------------

    //--------------------------- TIMER ------------------------------------
    if (!perspectiva_drona) {
        if (VerificaCheckPoint() == false) {  // verificam daca s-au parcurs toate check-pointurile
            // daca inca nu s-a trecut prin toate se scad secundele, altfel timpul SE OPRSTE
            if (time_secunde >= 0) {
                time_secunde = 90 - Engine::GetElapsedTime();  // schimbam in functie de cate minute vrem sa avem
            }
        }

        // Timerul propriu-zis
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(dronePosition.x, dronePosition.y, dronePosition.z));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(droneRotation2.x), glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(droneRotation2.y), glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(droneRotation2.z), glm::vec3(0, 0, 1));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(4.5, 3.5, -5)); // pozitionarea lui in fata camerei in dreapta 
        modelMatrix = glm::scale(modelMatrix, glm::vec3(time_secunde, 1.5f, 0.0f));
        RenderSimpleMesh(meshes["timer"], shaders["tema2"], modelMatrix, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));

        // CONTUR TIMER
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(dronePosition.x, dronePosition.y, dronePosition.z));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(droneRotation2.x), glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(droneRotation2.y), glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(droneRotation2.z), glm::vec3(0, 0, 1));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(4.5, 3.5, -5));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(90, 1.5f, 0.0f));
        RenderSimpleMesh(meshes["timercontur"], shaders["tema2"], modelMatrix, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    }

	// ---------------------- SCENA NOASTRA ----------------------
    // sistemul de coordonate + scena pe care v-a trebui sa o facem padurea 
    
    RenderScene(deltaTimeSeconds);
    DrawCoordinateSystem(); // le pastram pana cand fac tot background-ul 


	//----------------------- MINI VIEWPORT -----------------------
	// pentru mini view, golim bufferul de adancime ca se fie 2D si setam noua zona de desenare
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(miniViewportArea.x, miniViewportArea.y, miniViewportArea.width, miniViewportArea.height);
  
    

    RenderScene(deltaTimeSeconds);
    DrawCoordinateSystem(); // le pastram pana cand fac tot background-ul


	// -----------  SAGEATA DOAR PENTRU MINI VIEW ------------
    if (index < nr_checkpoints) {
        glm::vec3 dronaPozitie2D = glm::vec3(dronePosition.x, dronePosition.y, dronePosition.z);
        glm::vec3 nextCheckpointPos2D = glm::vec3(check_point_position[index].x, dronePosition.y, check_point_position[index].z);
        glm::vec3 direction = glm::normalize(nextCheckpointPos2D - dronaPozitie2D);

        // Desenarea sageata
        RenderArrow(dronaPozitie2D, direction);
    }

    // CAND ARATAM DRONA SE DUCE CU 5 IN FATA PE Z
    if (perspectiva_drona) {
        val_z_perspectiva = 5;
    }
    else {
        val_z_perspectiva = 0;
    }
   
}

// --------------- RENDER PENTRU SHEADER-UL TEMA2 ----------------
void Tema2::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, float noise, const glm::vec3& color)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    // Render an object using the specified shader and the specified position
    glUseProgram(shader->program);

    // TODO(student): Get shader location for uniform mat4 "Model"
    GLint modelLoc = glGetUniformLocation(shader->program, "model");

    // TODO(student): Set shader uniform "Model" to modelMatrix
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // TODO(student): Get shader location for uniform mat4 "View"
    GLint viewLoc = glGetUniformLocation(shader->program, "view");

    // TODO(student): Set shader uniform "View" to viewMatrix
    glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));


    // TODO(student): Get shader location for uniform mat4 "Projection"
    GLint projLoc = glGetUniformLocation(shader->program, "projection");

    // TODO(student): Set shader uniform "Projection" to projectionMatrix
    glm::mat4 projectionMatrix1 = GetSceneCamera()->GetProjectionMatrix();
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix1));

    // cand e activ noise -ul
    GLint applyNoiseLoc = glGetUniformLocation(shader->program, "applyNoise");
    glUniform1i(applyNoiseLoc, noise > 0.0f ? 1 : 0);

    // noise 
    GLint noiseLoc = glGetUniformLocation(shader->program, "noiseFactor");
    if (noiseLoc == -1) {
        std::cerr << "Error: noiseFactor uniform not found in shader.\n";
    }
    glUniform1f(noiseLoc, noise);

    // SCALAREA INALTIMII TERENUIL CAND ARE NOISE
    glUniform1f(glGetUniformLocation(shader->program, "heightScale"), 0.7f);  // pentru a seta cat de inalt vreau sa fie terenul dupa ce se aplica noise

	// culoare
    GLint objectColorLoc = glGetUniformLocation(shader->program, "object_color");
    glUniform3f(objectColorLoc, color.r, color.g, color.b);


    // Draw the object
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}

// ---------------------- RENDER PENTRU SHADER-UL UMBRA ----------------------
void Tema2::RenderUmbraMesh(Mesh* mesh, Shader* shadowShader, const glm::mat4& modelMatrix, const glm::vec3& shadowColor, const glm::vec3& lightDirection) {
    if (!mesh || !shadowShader || !shadowShader->GetProgramID())
        return;

    glUseProgram(shadowShader->program);

    // Setează matricile de transformare
    GLint modelLoc = glGetUniformLocation(shadowShader->program, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    GLint viewLoc = glGetUniformLocation(shadowShader->program, "view");
    glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    GLint projLoc = glGetUniformLocation(shadowShader->program, "projection");
    glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    // Setează culoarea umbrei
    GLint shadowColorLoc = glGetUniformLocation(shadowShader->program, "shadow_color");
    glUniform3f(shadowColorLoc, shadowColor.r, shadowColor.g, shadowColor.b);

    // Setează direcția luminii
    GLint lightDirLoc = glGetUniformLocation(shadowShader->program, "lightDirection");
    glUniform3f(lightDirLoc, lightDirection.x, lightDirection.y, lightDirection.z);

    // Desenează mesh-ul umbrei
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}



void Tema2::FrameEnd()
{
}



bool Tema2::CheckColiziuneCilindru(const glm::vec3& dronePos, const glm::vec3& objectPos, float radius, float height) {
    // Verificam distanta pe planul XZ (orizontal)
    glm::vec3 orizontalDronePos = glm::vec3(dronePos.x, 0, dronePos.z);
    glm::vec3 orizontalObjectPos = glm::vec3(objectPos.x, 0, objectPos.z);
    float orizontalDistanta = glm::length(orizontalDronePos - orizontalObjectPos);

    // Verificam daca drona este in raza cilindrului
    if (orizontalDistanta > (radius + armLength2)) {
        return false; // in afara razei pe plan orizontal
    }

    // Verificam inaltimea (vertical)
	float droneBottom = dronePos.y - 0.1f; // partea inferioara a dronei, 0.1f grosime drona
    float droneTop = dronePos.y + 0.1f;   // partea superioara a dronei
    float objectBottom = objectPos.y;            // partea inferioara a cilindrului
    float objectTop = objectPos.y + height;      // partea superioara a cilindrului

    // Daca drona se intersecteaza vertical cu cilindrul
    return !(droneTop < objectBottom || droneBottom > objectTop);
}


bool Tema2::CheckColiziuneBox(const glm::vec3& dronePos, const glm::vec3& obstacleCenter,
    const glm::vec3& droneDimensions, const glm::vec3& obstacleDimensions) {
    // Verificam pe fiecare axa X, Y, Z
    return (std::abs(dronePos.x - obstacleCenter.x) <= (droneDimensions.x + obstacleDimensions.x) / 2.0f) &&
        (std::abs(dronePos.y - obstacleCenter.y) <= (droneDimensions.y + obstacleDimensions.y) / 2.0f) &&
        (std::abs(dronePos.z - obstacleCenter.z) <= (droneDimensions.z + obstacleDimensions.z) / 2.0f);
}


void Tema2::OnInputUpdate(float deltaTime, int mods)
{
	// cat timp tastele sunt apasate mutam drona, dar si camera 
    if (!window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {

        // modificam directia in functi de rotatie, pentru ca atunci cand se roteste drona sa se miste in directia corecta
        glm::quat droneQuaternion = glm::quat(glm::radians(glm::vec3(
            droneRotation2.y,   // (rotatie în jurul axei Y)
            droneRotation2.x,   // (rotatie în jurul axei X)
            droneRotation2.z    // (rotatie în jurul axei Z)
        )));

        // Directii de baza in spatiul local
        glm::vec3 forward = glm::vec3(0, 0, -1); // Inainte
        glm::vec3 right = glm::vec3(1, 0, 0);   // Dreapta

		//Transforma directiile locale in spatiul global
        forward = glm::normalize(droneQuaternion * forward);
        right = glm::normalize(droneQuaternion * right);

        glm::vec3 potentialPosition = dronePosition; // pozitia potentiala a dronei. O PRIMESTE IN MOMENTUL IN CARE NU SE LOVESTE DE CEVA !!!

        float viteza_deplasare = 10.0f;

        if (window->KeyHold(GLFW_KEY_W)) potentialPosition.y += viteza_deplasare * deltaTime;
        if (window->KeyHold(GLFW_KEY_S)) potentialPosition.y -= viteza_deplasare * deltaTime;
        if (window->KeyHold(GLFW_KEY_RIGHT)) potentialPosition += right * viteza_deplasare * deltaTime;
        if (window->KeyHold(GLFW_KEY_LEFT)) potentialPosition -= right * viteza_deplasare * deltaTime;
        if (window->KeyHold(GLFW_KEY_UP)) potentialPosition += forward * viteza_deplasare * deltaTime;
        if (window->KeyHold(GLFW_KEY_DOWN)) potentialPosition -= forward * viteza_deplasare * deltaTime;

        // --------------------------- VERIFICARE COLIZIUNI -----------------------

        bool coliziune = false;

        // COLIZIUNE COPACI
        for (const auto& position : treePositions) {
            // raza cea mai mare este 3 la brad si inaltimea 9.5
			if (CheckColiziuneCilindru(potentialPosition, position, 3.0f, 9.5f + 0.7f)) {  // am translat la inceput cu 0.7 pentru a se vedea bine umbra
                coliziune = true;
                break;
            }
        }

		// COLIZIUNE CLADIRI
        for (const auto& position : buildingPositions) {
			// raza cladire este jumate din dimensiuni generale (15, 7, 8)
            glm::vec3 buildingDimensions(15.0f, 16.0f, 8.0f); // Dimensiunile cladirii
            glm::vec3 dronedimensiuni(1.3f, 0.1f, 0.1f);
            if (CheckColiziuneBox(potentialPosition, position, dronedimensiuni, buildingDimensions)) {
				coliziune = true;
                break;
            }
        }

        // COLIZIUNE TEREN
		if (potentialPosition.y < 0.2f)  // 0.1f noise + 0.1f grosime drona 
		{
			coliziune = true;
		}

		// COLIZIUNE CU CHECK-POINT-URILE
        for (const auto& obstacle : check_point_position) {
		    // 0.75f la z este jumate din lungimea bratului dronei, pentru a nu inainta cu corpul dronei in obiect
            // la x si y am caluclat in functie de componenetele check-pointului
           glm::vec3 dimensiunipara(5.0f, 6.6f, 1.3f);
           glm::vec3 dronedimensiuni(1.3f, 0.1f, 0.1f);
		   glm::vec3 ob1pos = glm::vec3(obstacle.x, obstacle.y, obstacle.z + 0.75f);

           // verificare parte de jos
           if (CheckColiziuneBox(potentialPosition, ob1pos, dronedimensiuni, dimensiunipara)) {
                coliziune = true;
                break;
           }

           glm::vec3 ob6pos = glm::vec3(obstacle.x, obstacle.y, obstacle.z - 0.75f);
		   // verificare parte de jos din spate cu - 0.75f la z
           if (CheckColiziuneBox(potentialPosition, ob6pos, dronedimensiuni, dimensiunipara)) {
               coliziune = true;
               break;
           }

           // verificare parte de sus
           glm::vec3 dimensiunipara2(5.0f, 1.0f, 1.3f);
           glm::vec3 ob2pos = glm::vec3(obstacle.x, obstacle.y + 3.3f + 2.3f, obstacle.z + 0.75f);
           if (CheckColiziuneBox(potentialPosition, ob2pos, dronedimensiuni, dimensiunipara2)) {
               coliziune = true;
               break;
           }

           glm::vec3 dimensiunipara5(5.0f, 1.0f, 1.3f);
           glm::vec3 ob5pos = glm::vec3(obstacle.x, obstacle.y + 3.3f + 2.3f, obstacle.z - 0.75f);
           // verificare parte de sus din spate 
           if (CheckColiziuneBox(potentialPosition, ob5pos, dronedimensiuni, dimensiunipara5)) {
               coliziune = true;
               break;
           }

           
           // verificare parte din stanga
           glm::vec3 dimensiunipara3(1.5f, 6.0f, 1.3f);
           glm::vec3 ob3pos = glm::vec3(obstacle.x - 2.2f, obstacle.y + 3.3f, obstacle.z + 0.75f);
           if (CheckColiziuneBox(potentialPosition, ob3pos, dronedimensiuni, dimensiunipara3)) {
               coliziune = true;
               break;
           }
           glm::vec3 ob7pos = glm::vec3(obstacle.x - 2.2f, obstacle.y + 3.3f, obstacle.z - 0.75f);
           // verificare parte din stanga din spate 
           if (CheckColiziuneBox(potentialPosition, ob7pos, dronedimensiuni, dimensiunipara3)) {
               coliziune = true;
               break;
           }

           // verificare parte din dreapta
           glm::vec3 dimensiunipara4(1.5f, 6.0f, 1.3f);
           glm::vec3 ob4pos = glm::vec3(obstacle.x + 2.2f, obstacle.y + 3.3f, obstacle.z + 0.75);
           if (CheckColiziuneBox(potentialPosition, ob4pos, dronedimensiuni, dimensiunipara4)) {
               coliziune = true;
               break;
           }

           glm::vec3 ob8pos = glm::vec3(obstacle.x + 2.2f, obstacle.y + 3.3f, obstacle.z - 0.75f);
           // verificare parte din dreapta din spate 
           if (CheckColiziuneBox(potentialPosition, ob8pos, dronedimensiuni, dimensiunipara4)) {
               coliziune = true;
               break;
           }
        }

        if (!coliziune) {
            dronePosition = potentialPosition;
        }

        // ROTATII
        float speed = 25.0f * deltaTime;
        if (window->KeyHold(GLFW_KEY_E))  droneRotation2.y += speed;
        if (window->KeyHold(GLFW_KEY_Q))  droneRotation2.y -= speed;
        if (window->KeyHold(GLFW_KEY_X))  droneRotation2.z += speed;
        if (window->KeyHold(GLFW_KEY_Z))  droneRotation2.z -= speed;
        if (window->KeyHold(GLFW_KEY_A)) droneRotation2.x += speed;
        if (window->KeyHold(GLFW_KEY_D)) droneRotation2.x -= speed;
    }

    // SHOW DRONA
	if (window->KeyHold(GLFW_KEY_1)) {
        if (perspectiva_drona == false) perspectiva_drona = true;
	}
	if(window->KeyHold(GLFW_KEY_2)) {
		if (perspectiva_drona == true) perspectiva_drona = false;
	}

}


void Tema2::OnKeyPress(int key, int mods)
{
    // Add key press event
    if (key == GLFW_KEY_SPACE)
    {
        switch (polygonMode)
        {
        case GL_POINT:
            polygonMode = GL_FILL;
            break;
        case GL_LINE:
            polygonMode = GL_POINT;
            break;
        default:
            polygonMode = GL_LINE;
            break;
        }
    }

}



void Tema2::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema2::OnWindowResize(int width, int height)
{
}
