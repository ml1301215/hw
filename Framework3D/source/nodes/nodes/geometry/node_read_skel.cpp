#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>

#include "GCore/Components/MaterialComponent.h"
#include "GCore/Components/MeshOperand.h"
#include "GCore/Components/XformComponent.h"
#include "Nodes/GlobalUsdStage.h"
#include "Nodes/node.hpp"
#include "Nodes/node_declare.hpp"
#include "Nodes/node_register.h"
#include "geom_node_base.h"
#include "pxr/base/gf/rotation.h"

#include <pxr/usd/usdSkel/cache.h>
#include <pxr/usd/usdSkel/skeleton.h>
#include <pxr/usd/usdSkel/skeletonQuery.h>
#include <pxr/usd/usdSkel/root.h>

#include "character_animation/animator.h"

namespace USTC_CG::node_character_animation {
static void node_declare(NodeDeclarationBuilder& b)
{
    b.add_input<decl::String>("File Name").default_val("Default");
    b.add_input<decl::String>("Prim Path").default_val("geometry");
    b.add_input<decl::Float>("Time Code").default_val(0).min(0).max(240);
    b.add_output<decl::Geometry>("Geometry");
}

static void node_exec(ExeParams params)
{
    auto file_name = params.get_input<std::string>("File Name");
    auto prim_path = params.get_input<std::string>("Prim Path");

    GOperandBase geometry;
    //std::shared_ptr<MeshComponent> mesh = std::make_shared<MeshComponent>(&geometry);
    //geometry.attach_component(mesh);

    auto t = params.get_input<float>("Time Code");
    pxr::UsdTimeCode time = pxr::UsdTimeCode(t);
    if (t == 0) {
        time = pxr::UsdTimeCode::Default();
    }

    auto stage = pxr::UsdStage::Open(file_name.c_str());

    Animator anim(stage, prim_path); 

    if (stage) {
        // Here 'c_str' call is necessary since prim_path
        auto sdf_path = pxr::SdfPath(prim_path.c_str());

        pxr::UsdSkelCache skelCache;
        pxr::UsdSkelSkeleton skel(stage->GetPrimAtPath(sdf_path));
        pxr::UsdSkelSkeletonQuery skelQuery = skelCache.GetSkelQuery(skel);

        if (skel) {
           /* for (size_t i = 0; i < skelQuery.GetJointOrder().size(); ++i) {
                pxr::SdfPath jointPath(skelQuery.GetJointOrder()[i]);
                std::cout << "Name of joint " << i << " is " << jointPath.GetName() << std::endl;
            }*/
        }
        else {
            std::cout << "No skel" << std::endl;
        }

        // TODO: add material reading
    }
    else {
        // TODO: throw something
        std::cout << "No stage" << std::endl;
    }
    params.set_output("Geometry", std::move(geometry));
}

static void node_register()
{
    static NodeTypeInfo ntype;

    strcpy(ntype.ui_name, "Read Skel");
    strcpy_s(ntype.id_name, "geom_read_skel");

    geo_node_type_base(&ntype);
    ntype.node_execute = node_exec;
    ntype.declare = node_declare;
    nodeRegisterType(&ntype);
}

NOD_REGISTER_NODE(node_register)
}  // namespace USTC_CG::node_character_animation
