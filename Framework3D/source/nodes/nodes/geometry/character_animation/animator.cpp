#include "animator.h"

namespace USTC_CG::node_character_animation {

using namespace pxr; 

Joint::Joint(string name, const Joint& parent) : name_(name), parent_(make_shared<Joint>(parent))
{
}

// In this function we build the tree of joints
SkeletonTree::SkeletonTree()
{
    size_t numJoints = skelQuery.GetTopology().GetNumJoints();
    for (size_t i = 0; i < numJoints; ++i) {
        std::string name = SdfPath(skelQuery.GetJointOrder()[i]).GetName();
        int parent = skelQuery.GetTopology().GetParent(i);
        if (parent >= 0) {
            Joint parentJoint = joints[parent];
            joints.push_back(Joint(name, parentJoint));
        }
        else {
            // Root joint
            joints.push_back(Joint(name));
        }
    }
}

// In this function, we mainly initialize the skeleton tree and the mesh
Animator::Animator(const UsdStageRefPtr& stage, string skel_path, vector<string> mesh_path)
{
    
	// Load the skeleton
	skel_ = UsdSkelSkeleton(stage->GetPrimAtPath(SdfPath(skel_path.c_str())));
	skelQuery_ = skelCache_.GetSkelQuery(skel_);

	skeleton_tree_ = SkeletonTree();

	if (skel_) {
		for (size_t i = 0; i < skelQuery_.GetJointOrder().size(); ++i) {
			pxr::SdfPath jointPath(skelQuery_.GetJointOrder()[i]);

			auto joint_name = jointPath.GetName();
			std::cout << "Name of joint " << i << " is " << jointPath.GetName() << std::endl;
                        jointPath.GetParentPath();  // get parent
			skelQuery_.GetTopology().GetParent(i);

			// Add joint to the tree, here we also need to set the parent/child relationship
			skeleton_tree_.add_joint(joint_name);
		}
	}
	else {
		std::cout << "No skel" << std::endl;
	}


	// Load the mesh
    for (auto path : mesh_path) {
		mesh = Mesh(path);
	}


}

void Animator::step()
{
    	// Update the skeleton
	skeleton.compute_world_transforms_for_each_joint();
	// Update the mesh, TODO: which argument to pass?
	mesh.update();

}


}