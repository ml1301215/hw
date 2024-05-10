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

namespace USTC_CG::node_character_animation {
static void node_declare(NodeDeclarationBuilder& b)
{
   /* b.add_input<decl::String>("File Name").default_val("Default");
    b.add_input<decl::String>("Prim Path").default_val("geometry");
    b.add_input<decl::Float>("Time Code").default_val(0).min(0).max(240);
    b.add_output<decl::Geometry>("Geometry");*/

    // Should read skel and mesh and time code 

    // output the deformed mesh 
}

static void node_exec(ExeParams params)
{
    auto time_code = params.get_input<float>("time_code");


    //auto mass_spring = params.get_input<std::shared_ptr<MassSpring>>("Mass Spring");

    // TODO: should create an animator class here 

    auto geometry = params.get_input<GOperandBase>("Mesh");
    auto mesh = geometry.get_component<MeshComponent>();
    if (mesh->faceVertexCounts.size() == 0)
    {
        throw std::runtime_error("Read USD error.");
    }


    if (time_code == 0) {  // If time = 0, reset and initialize the mass spring class
        if (mesh) {
            // Initialize the animator class here 
        }
        else {
            mass_spring = nullptr;
            throw std::runtime_error("Mass Spring: Need Geometry Input.");
        }
    }
    else  // otherwise, step forward the animation 
    {
        mass_spring->step(); 
    }

    // update mesh 

    mesh->vertices = eigen_to_usd_vertices(mass_spring->getX());

    params.set_output("Mass Spring Class", mass_spring);
    params.set_output("Output Mesh", geometry);
}

static void node_register()
{
    static NodeTypeInfo ntype;

    strcpy(ntype.ui_name, "Animate Mesh");
    strcpy_s(ntype.id_name, "geom_animate_mesh");

    geo_node_type_base(&ntype);
    ntype.node_execute = node_exec;
    ntype.declare = node_declare;
    nodeRegisterType(&ntype);
}

NOD_REGISTER_NODE(node_register)
}  // namespace USTC_CG::node_animate_mesh
