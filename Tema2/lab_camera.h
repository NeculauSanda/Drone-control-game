#pragma onces

#include "utils/glm_utils.h"
#include "utils/math_utils.h"


namespace implemented
{
    class CameraLab
    {
    public:
        CameraLab()
        {
            position = glm::vec3(0, 0, 3);
            forward = glm::vec3(0, 0, -1);
            up = glm::vec3(0, 1, 0);
            right = glm::vec3(1, 0, 0);
            distanceToTarget = 2.5f;
        }

        glm::mat4 RotationFromYawPitchRoll(float yaw, float pitch, float roll) {
            glm::mat4 rotationYaw = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0, 1, 0));
            glm::mat4 rotationPitch = glm::rotate(glm::mat4(1.0f), glm::radians(pitch), glm::vec3(1, 0, 0));
            glm::mat4 rotationRoll = glm::rotate(glm::mat4(1.0f), glm::radians(roll), glm::vec3(0, 0, 1));

            // Ordinea rota?iilor: yaw -> pitch -> roll
            return rotationYaw * rotationPitch * rotationRoll;
        }

        void SetPositionAndRotation(const glm::vec3& dronePosition, const glm::vec3& droneRotation) {
            // Drone's forward vector
            glm::mat4 rotationMatrix = RotationFromYawPitchRoll(glm::radians(droneRotation.y), glm::radians(droneRotation.x), glm::radians(droneRotation.z));
            forward = glm::normalize(glm::vec3(rotationMatrix * glm::vec4(0, 0, -1, 0)));
            right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
            up = glm::normalize(glm::cross(right, forward));

            // Set the camera position behind and above the drone
            position = dronePosition - forward * distanceToTarget + glm::vec3(0, 1, 0); // Adjust height
        }

        // Sets the camera position and orientation
        void Set(const glm::vec3& pos, const glm::vec3& center, const glm::vec3& upVec) {
            position = pos;
            forward = glm::normalize(center - pos);
            right = glm::cross(forward, upVec);
            up = glm::cross(right, forward);
        }

        // Translate the camera
        void TranslateForward(float distance) {
            position += forward * distance;
        }

        void TranslateRight(float distance) {
            position += right * distance;
        }

        void TranslateUpward(float distance) {
            position += up * distance;
        }

        // Rotate the camera
        void RotateFirstPerson_OX(float angle) {
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, right);
            forward = glm::normalize(glm::vec3(rotation * glm::vec4(forward, 0)));
            up = glm::normalize(glm::cross(right, forward));
        }

        void RotateFirstPerson_OY(float angle) {
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));
            forward = glm::normalize(glm::vec3(rotation * glm::vec4(forward, 0)));
            right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
            up = glm::cross(right, forward);
        }

        void RotateFirstPerson_OZ(float angle) {
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, forward);
            right = glm::normalize(glm::vec3(rotation * glm::vec4(right, 0)));
            up = glm::normalize(glm::cross(right, forward));
        }

        // Get the view matrix
        glm::mat4 GetViewMatrix() {
            return glm::lookAt(position, position + forward, up);
        }

        // Get the projection matrix
        glm::mat4 GetProjectionMatrix(float fov, float aspectRatio, float nearPlane, float farPlane) {
            return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
        }

        glm::vec3 GetPosition() const { return position; }

        glm::vec3 GetTargetPosition()
        {
            return position + forward * distanceToTarget;
        }
    private:
        glm::vec3 position;
        glm::vec3 forward;
        glm::vec3 up;
        glm::vec3 right;
        float distanceToTarget;
    };
} // namespace implemented