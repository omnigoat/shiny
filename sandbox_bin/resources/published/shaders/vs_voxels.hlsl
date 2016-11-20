struct vs_input_t
{
	float4 position : Position;
};

struct ps_input_t
{
	float4 actual_position : SV_Position;
	float3 view_dir : ViewDirection;
};

cbuffer buf_scene : register(b0)
{
	matrix view;
	matrix inverse_view;
	matrix proj;
	matrix inverse_vp;
	float time;
};

cbuffer buf_voxel : register(b2)
{
	float4 position;
	float yaw, pitch;
	uint brickcache_width;
	uint brick_size;
}


ps_input_t main(in vs_input_t input)
{
	// calculate view-directions for the vertices, so they interpolate across pixels
	float up_pitch = pitch - 3.1415 * 0.5f;

	float3 view_dir = {sin(yaw) * cos(pitch), -sin(pitch), cos(pitch) * cos(yaw)};
	float3 up       = {sin(yaw) * cos(up_pitch), -sin(up_pitch), cos(up_pitch) * cos(yaw)};
	float3 right    = cross(up, view_dir);

	float aspect  = proj[1][1] / proj[0][0];
	float inv_fov = 1.f / (atan(1.f / proj[0][0]) * 2.f);

	float3 ray = normalize(view_dir + up * input.position.y * inv_fov + right * input.position.x * aspect * inv_fov);

	
	ps_input_t output =
	{
		input.position,
		ray
	};

	return output;
}