#include "Skybox.h"
#include "Application.h"
#include "Module/ModuleTexture.h"
#include "Module/ModuleProgram.h"
#include "Component/ComponentCamera.h"

Skybox::Skybox() 
{
	LoadDefaultSkybox();
	GenerateSkyboxCube();
	
}

void Skybox::LoadDefaultSkybox()
{

	std::string cube_face_front_path = "resources/skyboxes/sor_lake1/lake1_ft.jpg";
	std::string cube_face_back_path = "resources/skyboxes/sor_lake1/lake1_bk.jpg";

	std::string cube_face_left_path = "resources/skyboxes/sor_lake1/lake1_lf.jpg";
	std::string cube_face_right_path = "resources/skyboxes/sor_lake1/lake1_rt.jpg";

	std::string cube_face_up_path = "resources/skyboxes/sor_lake1/lake1_up.jpg";
	std::string cube_face_down_path = "resources/skyboxes/sor_lake1/lake1_dn.jpg";

	std::vector<std::string> faces{
		cube_face_front_path,
		cube_face_back_path,
		cube_face_left_path,
		cube_face_right_path,
		cube_face_up_path,
		cube_face_down_path
	};
	skybox_texture = App->texture->LoadCubemap(faces);
}

void Skybox::GenerateSkyboxCube() 
{
	float skybox_vertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	glGenVertexArrays(1, &skybox_VAO);
	glGenBuffers(1, &skybox_VBO);
	glBindVertexArray(skybox_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, skybox_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), &skybox_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void Skybox::Render(const ComponentCamera & camera) const
{
	glDepthFunc(GL_LEQUAL);

	glUseProgram(App->program->skybox_program);

	glUniformMatrix4fv(
		glGetUniformLocation(App->program->skybox_program, "view"),
		1,
		GL_TRUE,
		&camera.GetViewMatrix()[0][0]
	);
	glUniformMatrix4fv(
		glGetUniformLocation(App->program->skybox_program, "proj"),
		1,
		GL_TRUE,
		&camera.GetProjectionMatrix()[0][0]
	);


	//Draw skybox
	glBindVertexArray(skybox_VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glDepthFunc(GL_LESS);
	glUseProgram(0);
}
Skybox::~Skybox()
{
	glDeleteTextures(1, &skybox_texture);
	glDeleteVertexArrays(1, &skybox_VAO);
	glDeleteBuffers(1, &skybox_VBO);
}