/**
 *  @example oglplus/020_shaded_objects.cpp
 *  @brief Shows how to draw several differently shaded geometric shapes
 *
 *  @image html 020_shaded_objects.png
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <oglplus/gl.hpp>
#include <oglplus/all.hpp>
#include <oglplus/shapes/cube.hpp>
#include <oglplus/shapes/sphere.hpp>
#include <oglplus/shapes/torus.hpp>

#include <cmath>

#include "example.hpp"

namespace oglplus {

// Renderable geometric shape built by the ShapeMaker class
template <class ShapeMaker>
class Shape
{
private:
	// helper object building shape's vertex attributes
	ShapeMaker make_shape;
	// helper object encapsulating the drawing instructions
	shapes::DrawingInstructions shape_instr;
	// indices pointing to the shape's primitive elements
	typename ShapeMaker::IndexArray shape_indices;

	// Fragment shader is owned by each individual shape
	FragmentShader fs;

	// Shading program
	Program prog;

	// A vertex array object for the rendered shape
	VertexArray shape;

	// VBOs for the shape's vertices, normals and tex-coordinates
	Buffer verts, normals, texcoords;
public:
	Shape(const VertexShader& vs, FragmentShader&& frag)
	 : shape_instr(make_shape.Instructions())
	 , shape_indices(make_shape.Indices())
	 , fs(std::forward<FragmentShader>(frag))
	{
		// attach the shaders to the program
		prog.AttachShader(vs);
		prog.AttachShader(fs);
		// link and use it
		prog.Link();
		prog.Use();

		// bind the VAO for the shape
		shape.Bind();

		const size_t n_attr = 3;
		// pointers to the vertex attribute data build functions
		typedef GLuint (ShapeMaker::*Func)(std::vector<GLfloat>&) const;
		Func func[n_attr] = {
			&ShapeMaker::Vertices,
			&ShapeMaker::Normals,
			&ShapeMaker::TexCoordinates
		};
		// managed references to the VBOs
		Managed<Buffer> vbo[n_attr] = {verts, normals, texcoords};
		// vertex attribute identifiers from the shaders
		const GLchar* ident[n_attr] = {"vertex", "normal", "texcoord"};

		for(size_t i=0; i!=n_attr; ++i)
		{
			// bind the VBO
			vbo[i].Bind(Buffer::Target::Array);
			// make the data
			std::vector<GLfloat> data;
			GLuint n_per_vertex = (make_shape.*func[i])(data);
			// upload the data
			Buffer::Data(Buffer::Target::Array, data);
			// setup the vertex attrib
			VertexAttribArray attr(prog, ident[i]);
			attr.Setup(n_per_vertex, DataType::Float);
			attr.Enable();
		}
	}

	void SetProjection(const Matrix4f& projection)
	{
		prog.Use();
		Uniform(prog, "projectionMatrix").SetMatrix(projection);
	}

	void Render(
		const Vec3f& light,
		const Matrix4f& camera,
		const Matrix4f& model
	)
	{
		// use the shading program
		prog.Use();
		// set the uniforms
		Uniform(prog, "lightPos").Set(light);
		Uniform(prog, "cameraMatrix").SetMatrix(camera);
		Uniform(prog, "modelMatrix").SetMatrix(model);
		// bind the VAO
		shape.Bind();
		// use the instructions to draw the shape
		// (this basically calls glDrawArrays* or glDrawElements*)
		shape_instr.Draw(shape_indices);
	}
};

class ShapeExample : public Example
{
private:
	// wrapper around the current OpenGL context
	Context gl;

	// The vertex shader is shader by the shapes
	VertexShader vs;

	// Makes the common shared vertex shader
	static VertexShader make_vs(void)
	{
		VertexShader shader;

		shader.Source(
			"#version 330\n"
			"uniform mat4 projectionMatrix, cameraMatrix, modelMatrix;"
			"in vec4 vertex;"
			"in vec3 normal;"
			"in vec2 texcoord;"
			"out vec2 fragTexCoord;"
			"out vec3 fragNormal;"
			"out vec3 fragLight;"
			"uniform vec3 lightPos;"
			"void main(void)"
			"{"
			"	fragTexCoord = texcoord;"
			"	fragNormal = ("
			"		modelMatrix *"
			"		vec4(normal, 0.0)"
			"	).xyz;"
			"	fragLight = ("
			"		vec4(lightPos, 0.0)-"
			"		modelMatrix*vertex"
			"	).xyz;"
			"	gl_Position = "
			"		projectionMatrix *"
			"		cameraMatrix *"
			"		modelMatrix *"
			"		vertex;"
			"}"
		);

		shader.Compile();
		return shader;
	}

	// The common first part of all fragment shader sources
	static const GLchar* fs_prologue(void)
	{
		return
		"#version 330\n"
		"in vec2 fragTexCoord;"
		"in vec3 fragNormal;"
		"in vec3 fragLight;"
		"out vec4 fragColor;"
		"void main(void)"
		"{"
		"	float _len = dot(fragLight, fragLight);"
		"	float _dot = max(dot(fragNormal, fragLight)/_len,0.0);"
		"	float intensity = clamp("
		"		0.2 + _dot * 2.0,"
		"		0.0,"
		"		1.0"
		"	);";
	}

	// The common last part of all fragment shader sources
	static const GLchar* fs_epilogue(void)
	{
		return
		"	fragColor = vec4(color * intensity, 1.0);"
		"}";
	}

	// The part calculating the color for the black/white checker shader
	static const GLchar* fs_bw_checker(void)
	{
		return
		"	float c = ("
		"		1 +"
		"		int(fragTexCoord.x*8) % 2+"
		"		int(fragTexCoord.y*8) % 2"
		"	) % 2;"
		"	vec3 color = vec3(c, c, c);";
	}

	// The part calculating the color for the yellow/black strips shader
	static const GLchar* fs_yb_strips(void)
	{
		return
		"	float m = int((fragTexCoord.x+fragTexCoord.y)*16) % 2;"
		"	vec3 color = mix("
		"		vec3(0.0, 0.0, 0.0),"
		"		vec3(1.0, 1.0, 0.0),"
		"		m"
		"	);";
	}

	// The part calculating the color for the white/orange strips shader
	static const GLchar* fs_wo_vstrips(void)
	{
		return
		"	float m = int(fragTexCoord.y*8) % 2;"
		"	vec3 color = mix("
		"		vec3(1.0, 0.6, 0.1),"
		"		vec3(1.0, 0.9, 0.8),"
		"		m"
		"	);";
	}

	// The part calculating the color for the blue/red circles shader
	static const GLchar* fs_br_circles(void)
	{
		return
		"	vec2  center = fragTexCoord - vec2(0.5, 0.5);"
		"	float m = int(sqrt(length(center))*16) % 2;"
		"	vec3 color = mix("
		"		vec3(1.0, 0.0, 0.0),"
		"		vec3(0.0, 0.0, 1.0),"
		"		m"
		"	);";
	}

	// The part calculating the color for the white/green spiral shader
	static const GLchar* fs_wg_spirals(void)
	{
		return
		"	vec2  center = (fragTexCoord - vec2(0.5, 0.5)) * 16.0;"
		"	float l = length(center);"
		"	float t = atan(center.y, center.x)/(2.0*asin(1.0));"
		"	float m = int(l+t) % 2;"
		"	vec3 color = mix("
		"		vec3(0.0, 1.0, 0.0),"
		"		vec3(1.0, 1.0, 1.0),"
		"		m"
		"	);";
	}

	// makes a fragment shader from the prologe, custom part and epilogue
	static FragmentShader make_fs(const char* color_fs)
	{
		FragmentShader shader;
		const GLchar* src[3] = {fs_prologue(), color_fs, fs_epilogue()};
		shader.Source(src, 3);
		shader.Compile();
		return shader;
	}

	// The shapes that will be rendered
	// a stationary sphere with unit radius at the origin
	Shape<shapes::Sphere> sphere;
	// three rotating unit cubes at the X, Y and Z axes
	Shape<shapes::Cube> cubeX, cubeY, cubeZ;
	// a torus rotating around the (1,1,1) vector
	Shape<shapes::Torus> torus;
public:
	ShapeExample(void)
	 : vs(make_vs())
	 , sphere(vs,make_fs(fs_yb_strips()))
	 , cubeX(vs, make_fs(fs_bw_checker()))
	 , cubeY(vs, make_fs(fs_br_circles()))
	 , cubeZ(vs, make_fs(fs_wg_spirals()))
	 , torus(vs, make_fs(fs_wo_vstrips()))
	{
		gl.ClearColor(0.5f, 0.5f, 0.5f, 0.0f);
		gl.ClearDepth(1.0f);
		gl.Enable(Capability::DepthTest);
	}

	void Reshape(size_t width, size_t height)
	{
		gl.Viewport(width, height);
		auto projection = CamMatrixf::Perspective(
			Degrees(24),
			double(width)/height,
			1, 100
		);
		sphere.SetProjection(projection);
		cubeX.SetProjection(projection);
		cubeY.SetProjection(projection);
		cubeZ.SetProjection(projection);
		torus.SetProjection(projection);
	}

	void Render(double time)
	{
		gl.Clear().ColorBuffer().DepthBuffer();
		//
		// make the light position vector
		auto light = Vec3f(2.0f, 2.0f, 2.0f);
		// make the matrix for camera orbiting the origin
		auto camera = CamMatrixf::Orbiting(
			Vec3f(),
			3.5,
			Degrees(time * 15),
			Degrees(std::sin(time) * 45)
		);
		// render the shapes
		sphere.Render(light, camera, Matrix4f());
		cubeX.Render(
			light,
			camera,
			ModelMatrixf::Translation(2.0f, 0.0f, 0.0f) *
			ModelMatrixf::RotationX(Degrees(time * 45))
		);
		cubeY.Render(
			light,
			camera,
			ModelMatrixf::Translation(0.0f, 2.0f, 0.0f) *
			ModelMatrixf::RotationY(Degrees(time * 90))
		);
		cubeZ.Render(
			light,
			camera,
			ModelMatrixf::Translation(0.0f, 0.0f, 2.0f) *
			ModelMatrixf::RotationZ(Degrees(time * 135))
		);
		torus.Render(
			light,
			camera,
			ModelMatrixf::Translation(-1.0f, -1.0f, -1.0f) *
			ModelMatrixf::RotationA(
				Vec3f(1.0f, 1.0f, 1.0f),
				Degrees(time * 45)
			) *
			ModelMatrixf::RotationY(Degrees(45)) *
			ModelMatrixf::RotationX(Degrees(45))
		);
	}

	bool Continue(double time)
	{
		return time < 30.0;
	}
};

std::unique_ptr<Example> makeExample(void)
{
	return std::unique_ptr<Example>(new ShapeExample);
}

} // namespace oglplus