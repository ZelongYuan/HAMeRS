#include "flow/flow_models/FlowModel.hpp"

/*
 * Check whether a patch is registered or not.
 */
bool
FlowModel::hasRegisteredPatch() const
{
    if (d_patch == nullptr)
    {
        return false;
    }
    
    return true;
}


/*
 * Get registered patch.
 */
const hier::Patch&
FlowModel::getRegisteredPatch() const
{
    if (d_patch == nullptr)
    {
        TBOX_ERROR(d_object_name
        << ": FlowModel::getRegisteredPatch()\n"
        << "Patch is not yet registered!"
        << std::endl);
    }
    
    return *d_patch;
}


/*
 * Return boost::shared_ptr to patch data context.
 */
const boost::shared_ptr<hier::VariableContext>&
FlowModel::getDataContext() const
{
    if (d_patch == nullptr)
    {
        TBOX_ERROR(d_object_name
        << ": FlowModel::getDataContext()\n"
        << "Patch is not yet registered!"
        << std::endl);
    }
    
   return d_data_context;
}


/*
 * Get sub-domain box.
 */
const hier::Box&
FlowModel::getSubdomainBox() const
{
    if (d_patch == nullptr)
    {
        TBOX_ERROR(d_object_name
        << ": FlowModel::getSubdomainBox()\n"
        << "Patch is not yet registered!"
        << std::endl);
    }
    
    return d_subdomain_box;
}


/*
 * Set sub-domain box.
 */
void
FlowModel::setSubdomainBox(const hier::Box& subdomain_box)
{
    if (d_patch == nullptr)
    {
        TBOX_ERROR(d_object_name
        << ": FlowModel::getSubdomainBox()\n"
        << "Patch is not yet registered!"
        << std::endl);
    }
    
    TBOX_ASSERT(d_ghost_box.contains(subdomain_box));
    
    d_subdomain_box = subdomain_box;
}


/*
 * Setup the Riemann solver object.
 */
void
FlowModel::setupRiemannSolver()
{
    d_flow_model_riemann_solver->setFlowModel(shared_from_this());
}


/*
 * Setup the basic utilties object.
 */
void
FlowModel::setupBasicUtilities()
{
    d_flow_model_basic_utilities->setFlowModel(shared_from_this());
}


/*
 * Setup the diffusive flux utilties object.
 */
void
FlowModel::setupDiffusiveFluxUtilities()
{
    d_flow_model_diffusive_flux_utilities->setFlowModel(shared_from_this());
}


/*
 * Setup the source utilties object.
 */
void
FlowModel::setupSourceUtilities()
{
    d_flow_model_source_utilities->setFlowModel(shared_from_this());
}


/*
 * Setup the statistics utilties object.
 */
void
FlowModel::setupStatisticsUtilities()
{
    d_flow_model_statistics_utilities->setFlowModel(shared_from_this());
}
