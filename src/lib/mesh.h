#ifndef MESH_H
#define MESH_H

#include "common.h"
#include "bvh.h"
#include "triangle.h"

#include<vector>
#include<unordered_map>

using std::vector;
using std::unordered_map;

class mesh_vertex{
  public:
	point3 position;
	vec3 normal;
	vec2 tex_coord;
	mesh_vertex(const point3& position=point3(0,0,0), const vec3& normal=vec3(0,0,0), const vec2& tex_coord=vec2(0,0))
		:position(position), normal(normal), tex_coord(tex_coord){}
};

class mesh: public hittable{
  public:
	mesh(const vector<mesh_vertex>& vertices, const vector<vec3i>& faces, const shared_ptr<material>& mat,
		 bool using_vertex_normals=false)
			:vertices(vertices), faces(faces), mat(mat), using_vertex_normals(using_vertex_normals){
		num_faces=faces.size(),num_vertices=vertices.size();
		hittable_list triangle_list;
		boundingbox=bounding_box::empty;
		for(int i=0;i<faces.size();i++){
			auto T=faces[i];
			point3 A=vertices[T.x].position,B=vertices[T.y].position,C=vertices[T.z].position;
			auto new_triangle=make_shared<triangle>(A,B,C,mat);
			triangle_id[new_triangle->get_pointer()]=i;
			triangle_list.add(new_triangle);
		}
		triangles=make_shared<bvh_node>(triangle_list);
		boundingbox=triangles->bbox();
	}

	bool hit(const ray& r, const interval& ray_t, hit_record& rec)const override{
		if(!triangles->hit(r,ray_t,rec))return 0;
		double v=rec.tex_coord.u, w=rec.tex_coord.v, u=1-v-w;
		auto hit_triangle=faces[triangle_id.at(rec.obj)];
		rec.tex_coord=u*vertices[hit_triangle.x].tex_coord
					 +v*vertices[hit_triangle.y].tex_coord
					 +w*vertices[hit_triangle.z].tex_coord;
		if(using_vertex_normals)
			rec.set_normal(r,normalize(u*vertices[hit_triangle.x].normal
					  				   +v*vertices[hit_triangle.y].normal
					  				   +w*vertices[hit_triangle.z].normal));
		return 1;
	}
	bounding_box bbox()const override{ return boundingbox;}
	double sample_pdf(const ray& r)const override{ return triangles->sample_pdf(r);}
    vec3 sample(const point3& origin,const double t)const override{ return triangles->sample(origin,t);}
    const void* get_pointer()const override{return this;}

  private:
	bool using_vertex_normals;
	int num_vertices,num_faces;
	vector<mesh_vertex> vertices;
	vector<vec3i> faces;
	shared_ptr<hittable> triangles;
	unordered_map<const void*, int> triangle_id;
	shared_ptr<material> mat;
	bounding_box boundingbox;
};

#endif