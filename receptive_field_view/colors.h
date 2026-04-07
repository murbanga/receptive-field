#pragma once

namespace Colors
{
#if 1
constexpr float inactive_field[] =
{ 0.7f, 0.7f, 0.7f, 0.5f };
constexpr float hovered_receptive_field[] =
{ 0.8f, 0.8f, 1.0f, 1.0f };
constexpr float hovered_affected_output[] =
{ 0.8f, 1.0f, 0.8f, 1.0f };
constexpr float hovered_pixel[] =
{ 1.0f, 1.0f, 0.8f, 1.0f };
constexpr float selected_tensor[] =
{ 1.0f, 1.0f, 0.2f, 1.0f };
constexpr float selected_pixel[] =
{ 1.0f, 1.0f, 0.5f, 1.0f };
constexpr float selected_receptive_field[] =
{ 0.5f, 0.5f, 0.6f, 1.0f};
constexpr float selected_affected_output[] =
{ 0.5f, 0.6f, 0.5f, 1.0f };
constexpr float background_color[] =
{ 0.1f,0.1f,0.1f, 1 };
#else
constexpr float inactive_field[] =
{ 0.7f, 0.7f, 0.7f, 0.5f };
constexpr float hovered_receptive_field[] =
{ 0.8f, 0.8f, 0.5f, 1.0f };
constexpr float hovered_affected_output[] =
{ 0.8f, 0.5f, 0.3f, 1.0f };
constexpr float hovered_pixel[] =
{ 1.0f, 1.0f, 0.8f, 1.0f };
constexpr float selected_tensor[] =
{ 1.0f, 1.0f, 0.2f, 1.0f };
constexpr float selected_pixel[] =
{ 1.0f, 1.0f, 0.5f, 1.0f };
constexpr float selected_receptive_field[] =
{ 0.5f, 0.5f, 0.6f, 1.0f };
constexpr float selected_affected_output[] =
{ 0.5f, 0.6f, 0.5f, 1.0f };
constexpr float background_color[] =
{ 1.0f,1.0f,1.0f, 1 };
#endif
}
