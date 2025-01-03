#include "common.h"
#include "hittable.h"
#include "sphere.h"
#include "hittable_list.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"
#include "texture.h"
#include "quad.h"
#include "make_box.h"
#include "transformations.h"
#include "mesh.h"
#include "loader.h"

void bouncing_spheres(){
    hittable_list scene, skybox;

    auto checker = make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));
    scene.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(checker)));
    skybox.add(make_shared<sphere>(point3(0,0,0),1e6,nullptr));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = random_color() * random_color();
                    sphere_material = make_shared<lambertian>(albedo);
                    auto center2 = center + vec3(0, random_double(0,0.15), 0);
                    scene.add(make_shared<sphere>(center, center2, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = random_color(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    scene.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    scene.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    scene.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    scene.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    scene.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 1200;
    cam.samples_per_pixel = 2000;
    cam.max_depth         = 30;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.position = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    cam.render(make_shared<bvh_node>(scene));
}
void checkered_spheres() {
    hittable_list world;

    auto checker = make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));

    world.add(make_shared<sphere>(point3(0,-10, 0), 10, make_shared<lambertian>(checker)));
    world.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 1200;
    cam.samples_per_pixel = 2000;
    cam.max_depth         = 50;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.position = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(make_shared<hittable_list>(world));
}
void earth() {
    auto earth_texture = make_shared<image_texture>("../../images/earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<sphere>(point3(0,0,0), 2, earth_surface);

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.position = point3(0,3,-12);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(make_shared<hittable_list>(globe));
}
void perlin_spheres() {
    hittable_list world;

    auto pertext = make_shared<grey_noise_texture>(4);
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));
    world.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertext)));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.position = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(make_shared<hittable_list>(world));
}
void quads() {
    hittable_list world;

    // Materials
    auto left_red     = make_shared<lambertian>(color(1.0, 0.2, 0.2));
    auto back_green   = make_shared<lambertian>(color(0.2, 1.0, 0.2));
    auto right_blue   = make_shared<lambertian>(color(0.2, 0.2, 1.0));
    auto upper_orange = make_shared<lambertian>(color(1.0, 0.5, 0.0));
    auto lower_teal   = make_shared<lambertian>(color(0.2, 0.8, 0.8));

    // Quads
    world.add(make_shared<quad>(point3(-3,-2, 5), vec3(0, 0,-4), vec3(0, 4, 0), left_red));
    world.add(make_shared<quad>(point3(-2,-2, 0), vec3(4, 0, 0), vec3(0, 4, 0), back_green));
    world.add(make_shared<quad>(point3( 3,-2, 1), vec3(0, 0, 4), vec3(0, 4, 0), right_blue));
    world.add(make_shared<quad>(point3(-2, 3, 1), vec3(4, 0, 0), vec3(0, 0, 4), upper_orange));
    world.add(make_shared<quad>(point3(-2,-3, 5), vec3(4, 0, 0), vec3(0, 0,-4), lower_teal));

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 80;
    cam.position = point3(0,0,9);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(make_shared<hittable_list>(world));
}
void simple_light() {
    hittable_list world;

    auto pertext = make_shared<grey_noise_texture>(4);
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));
    world.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertext)));

    auto difflight = make_shared<diffuse_light>(color(4,4,4));
    world.add(make_shared<sphere>(point3(0,7,0), 2, difflight));
    world.add(make_shared<quad>(point3(3,1,-2), vec3(2,0,0), vec3(0,2,0), difflight));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0,0,0);

    cam.vfov     = 20;
    cam.position = point3(26,3,6);
    cam.lookat   = point3(0,2,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(make_shared<hittable_list>(world));
}
void cornell_box() {
    hittable_list world, lights;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(20, 20, 20));
    auto blue  = make_shared<metal>(color(.75,.75,.95),0.03);
    auto yellow = make_shared<transparent>(make_shared<lambertian>(color(.9,.9,.3)),0.4);
    auto cyan = make_shared<lambertian>(color(.3,.9,.9));
    auto magenta = make_shared<metal>(color(.9,.3,.9),0.2);
    auto floor_texture=make_shared<image_texture>("../images/floor_texture.jpg");
    auto floor_surface=make_shared<lambertian>(floor_texture);

    auto blue_glass = make_shared<transparent>(blue,0.7);
    auto blue_glass2 = make_shared<transparent>(blue,0.3);

    world.add(make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
    world.add(make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
    world.add(make_shared<quad>(point3(343, 554, 332), vec3(-130,0,0), vec3(0,0,-105), light));
    lights.add(make_shared<quad>(point3(343, 554, 332), vec3(-130,0,0), vec3(0,0,-105), light));
    world.add(make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), floor_surface));
    world.add(make_shared<quad>(point3(555,555,555), vec3(-555,0,0), vec3(0,0,-555), white));
    world.add(make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), blue_glass));
    world.add(make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,555,0), blue_glass2));

    vector<mesh_vertex> vertices;
    vertices.push_back(mesh_vertex(point3(-69,0,40)));
    vertices.push_back(mesh_vertex(point3(69,0,40)));
    vertices.push_back(mesh_vertex(point3(0,0,-80)));
    vertices.push_back(mesh_vertex(point3(0,113,0)));
    vector<vec3i> faces;
    faces.push_back(vec3i(0,1,2));
    faces.push_back(vec3i(0,3,1));
    faces.push_back(vec3i(0,2,3));
    faces.push_back(vec3i(1,3,2));

    shared_ptr<hittable> mesh1=make_shared<mesh>(vertices,faces,magenta);
    mesh1=make_shared<rotate>(mesh1,0,-10,0);
    mesh1=make_shared<translate>(mesh1,vec3(450,0,100));
    world.add(mesh1);
    
    shared_ptr<hittable> box1 = make_box(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<rotate>(box1, 0, 15, 0);
    box1 = make_shared<translate>(box1, vec3(265,0,295));
    world.add(box1);

    auto glass = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(190,90,190), 90, glass));
    lights.add(make_shared<sphere>(point3(190,90,190), 90, glass));

    auto yellow_glass=make_shared<quad>(point3(150,200,150),vec3(80,0,0),vec3(0,0,80),yellow);
    world.add(yellow_glass);

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 30;
    cam.background        = color(0,0,0);

    cam.vfov     = 40;
    cam.position = point3(278, 278, -800);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(make_shared<bvh_node>(world),make_shared<hittable_list>(lights));
}
void assimp_test() {
    scene sponza;
    sponza.image_width=300;
    sponza.samples_per_pixel=100;
    sponza.max_depth=30;
    sponza.loadModel("CornellBox/cornellbox.glb");
    sponza.loadCamera("CornellBox/cornellbox.glb");

    sponza.render(0);
}

int main(int argc, char **argv) {
    srand(time(0));
    // Image
    char* OUT_FILE_PATH="output.ppm";
    int demo_id=1;

    for(int i=1;i<argc;i++){
        std::string arg=argv[i];
        if(arg=="-outfile"){
            if(i==argc-1){std::clog<<"Invalid arguments"<<std::endl;return -1;}
            OUT_FILE_PATH=argv[++i];
        }
        else if(arg=="-demo"){
            if(i==argc-1){std::clog<<"Invalid arguments"<<std::endl;return -1;}
            demo_id=std::stoi(argv[++i]);
        }
        else{std::clog<<"Invalid arguments"<<std::endl;return -1;}
    }
    freopen(OUT_FILE_PATH,"w",stdout);

    switch(demo_id){
        case 1: bouncing_spheres(); break;
        case 2: checkered_spheres(); break;
        case 3: earth(); break; 
        case 4: perlin_spheres(); break;
        case 5: quads(); break;
        case 6: simple_light(); break;
        case 7: cornell_box(); break;
        case 8: assimp_test(); break;
    }
    return 0;
}