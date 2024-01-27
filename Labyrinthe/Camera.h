#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class Camera {
public:
    Camera()
        : camera_pos(glm::vec3(0.0f, 0.0f, 8.0f)),
        camera_front(glm::vec3(0.0f, 0.0f, 1.0f)),
        camera_up(glm::vec3(0.0f, 1.0f, 0.0f)),
        camera_right(glm::vec3(1.0f, 0.0f, 0.0f)),
        mouse_sensitivity(0.25f),
        yaw(-90.0f),
        pitch(0.0f) {}

    glm::mat4 GetViewMatrix(glm::vec3 target_pos = glm::vec3(0.0f, -4.0f, -10.0f)) {
        if (target_pos != glm::vec3(0.0f)) {
            return glm::lookAt(camera_pos, target_pos + camera_front, camera_up);
        }
        else {
            return glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
        }
    }

    void ProcessMouseMovement(float xoffset, float yoffset, bool constrain_pitch) {
        xoffset *= mouse_sensitivity;
        yoffset *= mouse_sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (constrain_pitch) {
            if (pitch > 45.0f) {
                pitch = 45.0f;
            }
            if (pitch < -45.0f) {
                pitch = -45.0f;
            }
        }

        UpdateCameraVectors();
    }

    void ProcessKeyboard(const std::string& direction, float velocity) {
        if (direction == "FORWARD") {
            camera_pos += camera_front * velocity;
        }
        if (direction == "BACKWARD") {
            camera_pos -= camera_front * velocity;
        }
        if (direction == "LEFT") {
            camera_pos -= camera_right * velocity;
        }
        if (direction == "RIGHT") {
            camera_pos += camera_right * velocity;
        }
    }

private:
    glm::vec3 camera_pos;
    glm::vec3 camera_front;
    glm::vec3 camera_up;
    glm::vec3 camera_right;
    float mouse_sensitivity;
    float yaw;
    float pitch;

    void UpdateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        camera_front = glm::normalize(front);
        camera_right = glm::normalize(glm::cross(camera_front, glm::vec3(0.0f, 1.0f, 0.0f)));
        camera_up = glm::normalize(glm::cross(camera_right, camera_front));
    }
};

#endif // CAMERA_H_INCLUDED
