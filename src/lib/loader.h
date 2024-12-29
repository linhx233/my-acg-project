#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/material.h"
#include "common.h"
#include "camera.h"
#include "hittable_list.h"
#include "mesh.h"

#include<vector>

class scene{
  public:
    int image_width, samples_per_pixel, max_depth;
	std::vector<camera> cameras;
	std::vector<shared_ptr<hittable> > objects;
	std::vector<shared_ptr<hittable> > lights;
	std::vector<shared_ptr<material> > materials;
	std::vector<int> is_light;

	void render(int cam_id){
		auto object=make_shared<bvh_node>(objects,0,(int)objects.size());
		auto light=make_shared<hittable_list>(lights);
		cameras[cam_id].render(object,light);
	}

	bool loadCamera(const std::string &path){
		Assimp::Importer importer;
		const aiScene *scene=importer.ReadFile(path,aiProcess_Triangulate|aiProcess_PreTransformVertices);
		if(scene==nullptr||scene->mRootNode==nullptr||scene->mFlags&AI_SCENE_FLAGS_INCOMPLETE){
			std::clog<<"error::assimp::"<<importer.GetErrorString()<<std::endl;
			return 0;
		}
		
		if(scene->mNumCameras==0){
			std::clog<<"No camera objects!"<<std::endl;
			return 0;
		}
		for(int i=0;i<scene->mNumCameras;i++){
			const aiCamera *Cam=scene->mCameras[i];
			std::clog<<Cam->mName.C_Str()<<std::endl;
			camera cam;

			cam.aspect_ratio=Cam->mAspect;
			cam.image_width=image_width;
			cam.samples_per_pixel=samples_per_pixel; 
			cam.max_depth=max_depth;
			
			cam.position=Cam->mPosition;
			cam.lookat=Cam->mLookAt;
			cam.vfov=rad_to_deg(Cam->mHorizontalFOV);
			cam.vup=Cam->mUp;
			cam.defocus_angle=0;
			// cam.background=color(.7,.9,1);

			// std::clog<<"Aspect ratio: "<<cam.aspect_ratio<<std::endl;
			// std::clog<<"Image width: "<<cam.image_width<<std::endl;
			// std::clog<<"Spp: "<<cam.samples_per_pixel<<std::endl;
			// std::clog<<"Max depth: "<<cam.max_depth<<std::endl;

			std::clog<<"Position: "<<cam.position<<std::endl;
			std::clog<<"LookAt: "<<cam.lookat<<std::endl;
			std::clog<<"Vfov: "<<cam.vfov<<std::endl;
			std::clog<<"Vup: "<<cam.vup<<std::endl;
			// std::clog<<"Defocus angle: "<<cam.defocus_angle<<std::endl;
			
			cameras.push_back(cam);
		}
		return 1;
	}

	bool loadModel(const std::string& path){
		Assimp::Importer importer;
		const aiScene *scene=importer.ReadFile(path,aiProcess_Triangulate|aiProcess_PreTransformVertices);
		if(scene==nullptr||scene->mRootNode==nullptr||scene->mFlags&AI_SCENE_FLAGS_INCOMPLETE){
			std::clog<<"error::assimp::"<<importer.GetErrorString()<<std::endl;
			return 0;
		}

		for(int i=0;i<scene->mNumMaterials;i++){
			const aiMaterial *Mat=scene->mMaterials[i];int islight;
			materials.push_back(process_material(scene,Mat,islight));
			is_light.push_back(islight);
		}

		for(int i=0;i<scene->mNumMeshes;i++){
			const aiMesh *Mesh=scene->mMeshes[i];
			std::clog<<Mesh->mName.C_Str()<<' '<<Mesh->mNumFaces<<std::endl;

			vector<mesh_vertex> vertices;
			vector<vec3i> faces;
			shared_ptr<material> mat;
			bool using_vertex_normals=0;

			bool tex_coord_valid=0;
			if(Mesh->GetNumUVChannels()){
				if(Mesh->mNumUVComponents[0]!=2){
					std::clog<<"TexCoord only support two channels!"<<std::endl;
					continue;
				}
				tex_coord_valid=1;
			}
			if(Mesh->HasNormals())using_vertex_normals=1;

			for(int j=0;j<Mesh->mNumVertices;j++){
				mesh_vertex vertex;
				vertex.position=Mesh->mVertices[j];
				// std::clog<<vertex.position<<std::endl;
				if(using_vertex_normals)vertex.normal=Mesh->mNormals[j];
				if(tex_coord_valid)vertex.tex_coord=Mesh->mTextureCoords[0][j];
				vertices.push_back(vertex);
			}
			for(int j=0;j<Mesh->mNumFaces;j++){
				vec3i face=Mesh->mFaces[j];
				faces.push_back(face);
			}
			mat=materials[Mesh->mMaterialIndex];

			auto new_mesh=make_shared<mesh>(vertices,faces,mat,using_vertex_normals);

			objects.push_back(new_mesh);
			if(is_light[Mesh->mMaterialIndex])lights.push_back(new_mesh),std::clog<<"Is light!"<<std::endl;
		}

		for(int i=0;i<scene->mNumLights;i++){
			const aiLight *Light=scene->mLights[i];

			std::clog<<Light->mName.C_Str()<<std::endl;
			std::clog<<Light->mType<<std::endl;

			if(Light->mType==aiLightSource_DIRECTIONAL){
				vec3 dir=Light->mDirection;
				std::clog<<dir<<std::endl<<Light->mColorDiffuse<<std::endl;
				point3 p=dir*(-1e7);
				orthonormal_basis onb(dir);
				auto light=make_shared<quad>(p+onb.u+onb.v,-2*onb.u,-2*onb.v,
											 make_shared<diffuse_light>(Light->mColorDiffuse/25,1,0,0));
				objects.push_back(light);
				lights.push_back(light);
			}
		}

		if(lights.empty()){
			std::clog<<"No light!"<<std::endl;
		}

		return 1;
	}

  private:
	shared_ptr<material> process_material(const aiScene *Scene, const aiMaterial *Mat, int& islight){
		std::clog<<Mat->GetName().C_Str()<<std::endl;
		shared_ptr<material> mat;
		aiColor3D diffuse,specular,emitted;
		float opacity,roughness,refracti,metallic,emitted_intensity,transmission;
		aiString diffuse_texture,specular_texture,emitted_texture;
		auto has_diffuse_color=Mat->Get(AI_MATKEY_COLOR_DIFFUSE,diffuse);
		auto has_specular_color=Mat->Get(AI_MATKEY_COLOR_SPECULAR,specular);
		auto has_emitted_color=Mat->Get(AI_MATKEY_COLOR_EMISSIVE,emitted);
		auto has_emitted_intensity=Mat->Get(AI_MATKEY_EMISSIVE_INTENSITY,emitted_intensity);
		auto has_metallic=Mat->Get(AI_MATKEY_METALLIC_FACTOR,metallic);
		auto has_opacity=Mat->Get(AI_MATKEY_OPACITY,opacity);
		auto has_roughness=Mat->Get(AI_MATKEY_ROUGHNESS_FACTOR,roughness);
		auto has_refracti=Mat->Get(AI_MATKEY_REFRACTI,refracti);
		auto has_transmission=Mat->Get(AI_MATKEY_TRANSMISSION_FACTOR,transmission);
		auto has_diffuse_texture=Mat->GetTexture(aiTextureType_DIFFUSE,0,&diffuse_texture);
		auto has_specular_texture=Mat->GetTexture(aiTextureType_SPECULAR,0,&specular_texture);
		auto has_emitted_texture=Mat->GetTexture(aiTextureType_EMISSIVE,0,&emitted_texture);
		shared_ptr<texture> diffuse_tex,specular_tex,emitted_tex;
		if(has_diffuse_texture==AI_SUCCESS){
			auto embedded=Scene->GetEmbeddedTexture(diffuse_texture.C_Str());
			if(embedded!=nullptr)diffuse_tex=make_shared<image_texture>(embedded);
			else diffuse_tex=make_shared<image_texture>(diffuse_texture.C_Str());
			std::clog<<"Diffuse_texture: "<<diffuse_texture.C_Str()<<std::endl;
		}
		else if(has_diffuse_color==AI_SUCCESS){
			diffuse_tex=make_shared<solid_color>(color(diffuse));
			std::clog<<"Diffuse_color: "<<diffuse<<std::endl;
		}
		else diffuse_tex=make_shared<solid_color>(color(0,0,0));

		if(has_specular_texture==AI_SUCCESS){
			specular_tex=make_shared<solid_color>(color(0,0,0));
			// specular_tex=make_shared<image_texture>(specular_texture.C_Str());
			std::clog<<"Specular_texture: "<<specular_texture.C_Str()<<std::endl;
		}
		else if(has_specular_color==AI_SUCCESS){
			specular_tex=make_shared<solid_color>(color(0,0,0));
			// specular_tex=make_shared<solid_color>(color(specular));
			std::clog<<"Specular_color: "<<specular<<std::endl;
		}
		else specular_tex=make_shared<solid_color>(color(0,0,0));

		if(has_emitted_texture==AI_SUCCESS){
			auto embedded=Scene->GetEmbeddedTexture(emitted_texture.C_Str());
			if(embedded!=nullptr)diffuse_tex=make_shared<image_texture>(embedded);
			else emitted_tex=make_shared<image_texture>(emitted_texture.C_Str());
			islight=1;
			std::clog<<"Emitted_texture: "<<emitted_texture.C_Str()<<std::endl;
		}
		else if(has_emitted_color==AI_SUCCESS){
			emitted_tex=make_shared<solid_color>(color(emitted));
			islight=(length(emitted)>err);
			if(islight)std::clog<<"Emitted_color: "<<emitted<<std::endl;
		}
		else emitted_tex=make_shared<solid_color>(color(0,0,0)),islight=0;
		
		if(has_metallic!=AI_SUCCESS)metallic=0;
		if(has_roughness!=AI_SUCCESS)roughness=1;
		if(has_opacity!=AI_SUCCESS)opacity=1;
		if(has_refracti!=AI_SUCCESS)refracti=1.5;
		if(has_emitted_intensity!=AI_SUCCESS)emitted_intensity=1;
		if(has_transmission!=AI_SUCCESS)transmission=0;
		std::clog<<"Roughness: "<<roughness<<std::endl<<"Metallic: "<<metallic<<std::endl<<"Opacity: "<<opacity<<std::endl<<"IOR: "<<refracti<<"Transmission: "<<transmission<<std::endl;

		mat=make_shared<PrincipledBSDF>(diffuse_tex,nullptr,emitted_tex,emitted_intensity,roughness,metallic,opacity,refracti,transmission);
		return mat;
	}
};