#pragma once

#include <memory>
#include <vector>
#include <iostream>

#include <pxr/usd/usdSkel/cache.h>
#include <pxr/usd/usdSkel/skeleton.h>
#include <pxr/usd/usdSkel/skeletonQuery.h>
#include <pxr/usd/usdSkel/root.h>
#include <pxr/usd/usdSkel/animQuery.h>
#include <pxr/usd/usdSkel/animation.h>
#include <pxr/usd/usdSkel/binding.h>
#include <pxr/usd/usdGeom/mesh.h>

#include <Eigen/Dense>

namespace USTC_CG::node_character_animation {

using namespace std;
using namespace Eigen;
using namespace pxr; 

class Joint
{

public:
	// Need to give a default value for parent 
	// since root joint may have no parent 
    Joint(int idx, string name, int parent_idx); 

	int idx_;
    std::string name_;
	
	// One joint can have multiple children but one parent?
	//vector<shared_ptr<Joint>> parents_;
    shared_ptr<Joint> parent_;
    vector<shared_ptr<Joint>> children_;

	Matrix4d local_transform_;
    Matrix4d world_transform_;
};

// A tree of joints, similar to particle_system 
class SkeletonTree
{
   public:
    SkeletonTree();

    // root joint
    shared_ptr<Joint> root_;

    // All joints
    vector<shared_ptr<Joint>> joints_;

	// Students should call this function 
	void compute_world_transforms_for_each_joint(); 


	void add_joint(int idx, std::string name, int parent_idx); 


    // some transforms here

    // some functions to update the skeleton: add joints into the tree, access leaves
};

class Mesh
{
   public:
	 // update each vertex with a transform ? 
     //void update(); 

   protected:
	 MatrixXd vertices_; 
	 //some_type vertices_weight; 
	   // vertices, Weights of each joint 
};

class Animator{

public: 
	Animator() = default;
	~Animator() = default;

	// Load skeleton and mesh from a usd file 
	// What if there are multiply meshes for a skel?

	// This design is not good, but we can fix it later
	// Animator should not handle the reading of USD stage 
	Animator(const UsdStageRefPtr& stage, string root_path); 

	// Each timestep, update the skeleton transform,
	// then apply transform to each vertices of mesh 
	void step(); 

protected:
	// Here we need to manage the skeleton and mesh

	// joints hierarchy here?
	SkeletonTree skeleton_tree_; 

	// mesh member
	Mesh mesh_; 

	UsdSkelCache skelCache_; 
	UsdSkelSkeleton skel_;
	UsdSkelSkeletonQuery skelQuery_; 
};

}