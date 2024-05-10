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

Animator::Animator(string skel_path, vector<string> mesh_path)
{
    
    auto stage = pxr::UsdStage::Open(file_name.c_str());
	// Load the skeleton
	UsdStageRefPtr stage = UsdStage::Open(skel_path);
        pxr::UsdSkelCache skelCache;
        pxr::UsdSkelSkeleton skel(stage->GetPrimAtPath(sdf_path));
        pxr::UsdSkelSkeletonQuery skelQuery = skelCache.GetSkelQuery(skel);

    
    skel_ = UsdSkelSkelton.Get((stage, SdfPath("/Path/To/Skel"));

	UsdSkelRoot skelRoot(stage->GetPrimAtPath(SdfPath("/root")));
	skelQuery = UsdSkelSkeletonQuery(skelRoot.GetSkeleton());
	skeleton = SkeletonTree();
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