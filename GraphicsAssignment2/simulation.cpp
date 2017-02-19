#include "simulation.h"

#include "scene.h"

#include "imgui.h"

#define FLYTHROUGH_CAMERA_IMPLEMENTATION
#include "flythrough_camera.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SDL.h>



// Keep track of most recent transform of the moving teapot so we can add to it
uint32_t rotationTransformId;
uint32_t rotationParentTransformId;

void Simulation::Init(Scene* scene)
{
    mScene = scene;

    std::vector<uint32_t> loadedMeshIDs;

    loadedMeshIDs.clear();
    LoadMeshesFromFile(mScene, "assets/cube/cube.obj", &loadedMeshIDs);
    for (uint32_t loadedMeshID : loadedMeshIDs)
    {
        uint32_t newInstanceID;
        AddMeshInstance(mScene, loadedMeshID, &newInstanceID);

        // scale up the cube
        uint32_t newTransformID = scene->Instances[newInstanceID].TransformID;
        scene->Transforms[newTransformID].Scale = glm::vec3(2.0f);
    }

	{
		// Create a transform for rotating a teapot
		Transform newTransform;
		// This is a root node transform for now
		newTransform.ParentID = -1;
		newTransform.Scale = glm::vec3(1.0f);
		rotationTransformId = scene->Transforms.insert(newTransform);
	}

    loadedMeshIDs.clear();
    LoadMeshesFromFile(mScene, "assets/teapot/teapot.obj", &loadedMeshIDs);
    for (uint32_t loadedMeshID : loadedMeshIDs)
    {

        // place a teapot on top of the cube
        {
            uint32_t newInstanceID;
            AddMeshInstance(mScene, loadedMeshID, &newInstanceID);
            uint32_t newTransformID = scene->Instances[newInstanceID].TransformID;
            scene->Transforms[newTransformID].Translation += glm::vec3(0.0f, 2.0f, 0.0f);
        }

        // place a teapot on the side
        {
            uint32_t newInstanceID;
            AddMeshInstance(mScene, loadedMeshID, &newInstanceID);
			uint32_t newTransformID = scene->Instances[newInstanceID].TransformID;
            scene->Transforms[newTransformID].Translation += glm::vec3(3.0f, 1.0f, 4.0f);
			
			// We want to orbit this teapot, so save the transform id of one of it's parts
			rotationParentTransformId = newTransformID;
        }

        // place another teapot on the side
        {
            uint32_t newInstanceID;
            AddMeshInstance(mScene, loadedMeshID, &newInstanceID);
            uint32_t newTransformID = scene->Instances[newInstanceID].TransformID;

			// Make these transforms children of the rotation transform
			scene->Transforms[newTransformID].ParentID = rotationTransformId;

			// Don't translate this teapot up one, since it'll inherit the translation up from the above teapot
            scene->Transforms[newTransformID].Translation += glm::vec3(0.0f, 0.0f, -3.0f);
        }
    }

	// Set the parent of our roation transform to be the one we saved from the other teapot.
	mScene->Transforms[rotationTransformId].ParentID = rotationParentTransformId;

    loadedMeshIDs.clear();
    LoadMeshesFromFile(mScene, "assets/floor/floor.obj", &loadedMeshIDs);
    for (uint32_t loadedMeshID : loadedMeshIDs)
    {
        AddMeshInstance(mScene, loadedMeshID, nullptr);
    }

    Camera mainCamera;
    mainCamera.Eye = glm::vec3(5.0f);
    glm::vec3 target = glm::vec3(0.0f);
    mainCamera.Look = normalize(target - mainCamera.Eye);
    mainCamera.Up = glm::vec3(0.0f, 1.0f, 0.0f);
    mainCamera.FovY = glm::radians(70.0f);
    mScene->MainCamera = mainCamera;
}

void Simulation::HandleEvent(const SDL_Event& ev)
{
    if (ev.type == SDL_MOUSEMOTION)
    {
        mDeltaMouseX += ev.motion.xrel;
        mDeltaMouseY += ev.motion.yrel;
    }
}

void Simulation::Update(float deltaTime)
{
    const Uint8* keyboard = SDL_GetKeyboardState(NULL);
    
    int mx, my;
    Uint32 mouse = SDL_GetMouseState(&mx, &my);

    if ((mouse & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0)
    {
        flythrough_camera_update(
            value_ptr(mScene->MainCamera.Eye),
            value_ptr(mScene->MainCamera.Look),
            value_ptr(mScene->MainCamera.Up),
            NULL,
            deltaTime,
            5.0f, // eye_speed
            0.1f, // degrees_per_cursor_move
            80.0f, // max_pitch_rotation_degrees
            mDeltaMouseX, mDeltaMouseY,
            keyboard[SDL_SCANCODE_W], keyboard[SDL_SCANCODE_A], keyboard[SDL_SCANCODE_S], keyboard[SDL_SCANCODE_D],
            keyboard[SDL_SCANCODE_SPACE], keyboard[SDL_SCANCODE_LCTRL],
            0);
    }

    mDeltaMouseX = 0;
    mDeltaMouseY = 0;


	// Update the transform to orbit the other teapot
	mScene->Transforms[rotationTransformId].Rotation = glm::rotate(mScene->Transforms[rotationTransformId].Rotation, glm::radians(1.0f), glm::vec3(0, 1, 0));

	// Calculate absolute transforms
	for (uint32_t instanceID : mScene->Instances) {
		Instance* instance = &mScene->Instances[instanceID];
		Transform* transform = &mScene->Transforms[instance->TransformID];

		glm::mat4 absoluteTransform;
		absoluteTransform = translate(-transform->RotationOrigin) * absoluteTransform;
		absoluteTransform = mat4_cast(transform->Rotation) * absoluteTransform;
		absoluteTransform = translate(transform->RotationOrigin) * absoluteTransform;
		absoluteTransform = scale(transform->Scale) * absoluteTransform;
		absoluteTransform = translate(transform->Translation) * absoluteTransform;

		uint32_t parentId = transform->ParentID;
		while (parentId != -1) {
			Transform* parentTransform = &mScene->Transforms[parentId];

			absoluteTransform = translate(-parentTransform->RotationOrigin) * absoluteTransform;
			absoluteTransform = mat4_cast(parentTransform->Rotation) * absoluteTransform;
			absoluteTransform = translate(parentTransform->RotationOrigin) * absoluteTransform;
			absoluteTransform = scale(parentTransform->Scale) * absoluteTransform;
			absoluteTransform = translate(parentTransform->Translation) * absoluteTransform;

			parentId = parentTransform->ParentID;
		}

		// Save the absolute transform
		transform->absoluteTransform = absoluteTransform;
	}



	if (ImGui::Begin("Example GUI Window"))
	{
		ImGui::Text("Mouse Pos: (%d, %d)", mx, my);
		//ImGui::Text("Delta: (%f)", delta);
	}
	ImGui::End();
}

void* Simulation::operator new(size_t sz)
{
    // zero out the memory initially, for convenience.
    void* mem = ::operator new(sz);
    memset(mem, 0, sz);
    return mem;
}
    