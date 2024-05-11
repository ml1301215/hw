#include "animator.h"

namespace USTC_CG::node_character_animation {

using namespace pxr; 

Joint::Joint(int idx, string name, int parent_idx){
}

// In this function we build the tree of joints
SkeletonTree::SkeletonTree()
{
}

void SkeletonTree::add_joint(int idx, std::string name, int parent_idx)
{
	auto joint = make_shared<Joint>(idx, name, parent_idx);
	joints_.push_back(joint);
	if (parent_idx < 0) {
		root_ = joint;
	}
	else {
		joints_[parent_idx]->children_.push_back(joint);

        if (parent_idx < joints_.size())
			joint->parent_ = joints_[parent_idx];
		else
		{
            std::cout << "!!!!! parent_idx out of range" << std::endl;
			exit(1);
		}

	}
}

void SkeletonTree::compute_world_transforms_for_each_joint()
{
}

// In this function, we mainly initialize the skeleton tree and the mesh
Animator::Animator(const UsdStageRefPtr& stage, string root_path)
{
    UsdPrim skelRootPrim = stage->GetPrimAtPath(SdfPath(root_path.c_str())); 
	if (!skelRootPrim) {
        std::cout << "Failed to find SkelRoot prim." << endl;
        return;
    }
	UsdSkelRoot skelRoot(skelRootPrim); // TODO: seems useless? 


	// Load the skeleton
    string skel_path = skelRoot.GetPath().GetString() + "/Skel";
	skel_ = UsdSkelSkeleton(stage->GetPrimAtPath(SdfPath(skel_path.c_str())));

	skeleton_tree_ = SkeletonTree();

	if (skel_) {
		std::cout << "Load skel from Path: " << skel_path << std::endl;

		skelQuery_ = skelCache_.GetSkelQuery(skel_);

		for (size_t i = 0; i < skelQuery_.GetJointOrder().size(); ++i) {
			pxr::SdfPath jointPath(skelQuery_.GetJointOrder()[i]);

			auto joint_name = jointPath.GetName();
               std::cout << "Name of joint " << i << " is " << jointPath.GetName(); 
                        jointPath.GetParentPath();  // get parent

			int parent_idx = skelQuery_.GetTopology().GetParent(i);
            if (parent_idx < 0)
                  std::cout << "root " << std::endl;
            else {
                auto parent_name = SdfPath(skelQuery_.GetJointOrder()[parent_idx]).GetName();
                std::cout << "parent idx: " << parent_idx << "name: " << parent_name << std::endl;
            }


			// Add joint to the tree, here we also need to set the parent/child relationship
			skeleton_tree_.add_joint(i, joint_name, parent_idx);
		}
	}
	else {
		std::cout << "No skel at Path: " << skel_path << std::endl;
	}

	// get anim path from stage:
    auto anim_path = skel_.GetPath().GetString() + "/Anim";
	auto anim = UsdSkelAnimation(stage->GetPrimAtPath(SdfPath(anim_path.c_str())));
	if (anim)
	{
		std::cout << "Load anim from Path: " << anim_path << std::endl;
		UsdSkelAnimQuery animQuery = skelCache_.GetAnimQuery(anim);
	}
	else {
		std::cout << "No anim at Path: " << anim_path << std::endl;
	}

	// Load the mesh
    //UsdGeomMesh usdgeom(stage->GetPrimAtPath(sdf_path));



}

void Animator::step()
{
    	// Update the skeleton
	//skeleton.compute_world_transforms_for_each_joint();
	//// Update the mesh, TODO: which argument to pass?
	//mesh.update();

}


}