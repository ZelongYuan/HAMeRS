#include "flow/flow_models/five-eqn_Allaire/FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire.hpp"

/*
 * Register the required variables for the computation of diffusive fluxes in the registered patch.
 */
void
FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::registerDiffusiveFluxes(const hier::IntVector& num_subghosts)
{
    if (d_flow_model.expired())
    {
        TBOX_ERROR(d_object_name
            << ": "
            << "The object is not setup yet!"
            << std::endl);
    }
    
    boost::shared_ptr<FlowModel> flow_model_tmp = d_flow_model.lock();
    const hier::Patch& patch = flow_model_tmp->getRegisteredPatch();
    
    /*
     * Get the interior box.
     */
    
    const hier::Box interior_box = patch.getBox();
    
    /*
     * Register the required derived variables in flow model.
     */
    
    std::unordered_map<std::string, hier::IntVector> num_subghosts_of_data;
    
    num_subghosts_of_data.insert(
        std::pair<std::string, hier::IntVector>("MASS_FRACTIONS", num_subghosts));
    
    num_subghosts_of_data.insert(
        std::pair<std::string, hier::IntVector>("VELOCITY", num_subghosts));
    
    num_subghosts_of_data.insert(
        std::pair<std::string, hier::IntVector>("PRESSURE", num_subghosts));
    
    num_subghosts_of_data.insert(
        std::pair<std::string, hier::IntVector>("SPECIES_TEMPERATURES", num_subghosts));
    
    flow_model_tmp->registerDerivedVariables(num_subghosts_of_data);
    
    /*
     * Set ghost boxes of derived cell variables for this class.
     */
    
    d_num_subghosts_diffusivities = num_subghosts;
    d_subghost_box_diffusivities = interior_box;
    d_subghost_box_diffusivities.grow(d_num_subghosts_diffusivities);
    d_subghostcell_dims_diffusivities = d_subghost_box_diffusivities.numberCells();
    
    d_num_subghosts_shear_viscosity      = d_num_subghosts_diffusivities;
    d_num_subghosts_bulk_viscosity       = d_num_subghosts_diffusivities;
    
    d_subghost_box_shear_viscosity      = d_subghost_box_diffusivities;
    d_subghost_box_bulk_viscosity       = d_subghost_box_diffusivities;
    
    d_subghostcell_dims_shear_viscosity      = d_subghostcell_dims_diffusivities;
    d_subghostcell_dims_bulk_viscosity       = d_subghostcell_dims_diffusivities;
}


/*
 * The cell data of all derived variables in the patch for this class are dumped.
 */
void
FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::clearData()
{
    if (d_flow_model.expired())
    {
        TBOX_ERROR(d_object_name
            << ": "
            << "The object is not setup yet!"
            << std::endl);
    }
    
    boost::shared_ptr<FlowModel> flow_model_tmp = d_flow_model.lock();
    
    // Check whether a patch is already registered.
    if (!flow_model_tmp->hasRegisteredPatch())
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::"
            << "clearData()\n"
            << "No patch is registered yet."
            << std::endl);
    }
    
    d_num_subghosts_diffusivities   = -hier::IntVector::getOne(d_dim);
    d_num_subghosts_shear_viscosity = -hier::IntVector::getOne(d_dim);
    d_num_subghosts_bulk_viscosity  = -hier::IntVector::getOne(d_dim);
    
    d_subghost_box_diffusivities   = hier::Box::getEmptyBox(d_dim);
    d_subghost_box_shear_viscosity = hier::Box::getEmptyBox(d_dim);
    d_subghost_box_bulk_viscosity  = hier::Box::getEmptyBox(d_dim);
    
    d_subghostcell_dims_diffusivities   = hier::IntVector::getZero(d_dim);
    d_subghostcell_dims_shear_viscosity = hier::IntVector::getZero(d_dim);
    d_subghostcell_dims_bulk_viscosity  = hier::IntVector::getZero(d_dim);
    
    d_data_diffusivities.reset();
    d_data_shear_viscosity.reset();
    d_data_bulk_viscosity.reset();
    
    d_derived_cell_data_computed = false;
}


/*
 * Get the variables for the derivatives in the diffusive fluxes.
 */
void
FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxVariablesForDerivative(
    std::vector<std::vector<boost::shared_ptr<pdat::CellData<double> > > >& derivative_var_data,
    std::vector<std::vector<int> >& derivative_var_component_idx,
    const DIRECTION::TYPE& flux_direction,
    const DIRECTION::TYPE& derivative_direction)
{
    if (d_flow_model.expired())
    {
        TBOX_ERROR(d_object_name
            << ": "
            << "The object is not setup yet!"
            << std::endl);
    }
    
    boost::shared_ptr<FlowModel> flow_model_tmp = d_flow_model.lock();
    
    derivative_var_data.resize(d_num_eqn);
    derivative_var_component_idx.resize(d_num_eqn);
    
    // Get the cell data of velocity.
    boost::shared_ptr<pdat::CellData<double> > data_velocity =
        flow_model_tmp->getCellData("VELOCITY");
    
    if (d_dim == tbox::Dimension(1))
    {
        switch (flux_direction)
        {
            case DIRECTION::X_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species][0] = data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 0;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 1][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 0;
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxVariablesForDerivative()\n"
                            << "There are only x-direction for one-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxVariablesForDerivative()\n"
                    << "There are only x-direction for one-dimensional problem."
                    << std::endl);
            }
        }
    }
    else if (d_dim == tbox::Dimension(2))
    {
        switch (flux_direction)
        {
            case DIRECTION::X_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species][0] = data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 0;
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 1][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 1;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 2].resize(2);
                        derivative_var_component_idx[d_num_species + 2].resize(2);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 2][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 0;
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 2][1] = data_velocity;
                        derivative_var_component_idx[d_num_species + 2][1] = 1;
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species][0] = data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 1;
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 1][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 0;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 2].resize(2);
                        derivative_var_component_idx[d_num_species + 2].resize(2);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 2][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 0;
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 2][1] = data_velocity;
                        derivative_var_component_idx[d_num_species + 2][1] = 1;
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxVariablesForDerivative()\n"
                            << "There are only x-direction and y-direction for two-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            case DIRECTION::Y_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species][0] = data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 1;
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 1][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 0;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 2].resize(2);
                        derivative_var_component_idx[d_num_species + 2].resize(2);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 2][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 0;
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 2][1] = data_velocity;
                        derivative_var_component_idx[d_num_species + 2][1] = 1;
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species][0] = data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 0;
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 1][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 1;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 2].resize(2);
                        derivative_var_component_idx[d_num_species + 2].resize(2);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 2][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 0;
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 2][1] = data_velocity;
                        derivative_var_component_idx[d_num_species + 2][1] = 1;
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxVariablesForDerivative()\n"
                            << "There are only x-direction and y-direction for two-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxVariablesForDerivative()\n"
                    << "There are only x-direction and y-direction for two-dimensional problem."
                    << std::endl);
            }
        }
    }
    else if (d_dim == tbox::Dimension(3))
    {
        switch (flux_direction)
        {
            case DIRECTION::X_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species][0] = data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 0;
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 1][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 1;
                        
                        derivative_var_data[d_num_species + 2].resize(1);
                        derivative_var_component_idx[d_num_species + 2].resize(1);
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 2][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 2;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 3].resize(3);
                        derivative_var_component_idx[d_num_species + 3].resize(3);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 3][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][0] = 0;
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 3][1] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][1] = 1;
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 3][2] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][2] = 2;
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species][0] = data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 1;
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 1][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 0;
                        
                        derivative_var_data[d_num_species + 2].resize(0);
                        derivative_var_component_idx[d_num_species + 2].resize(0);
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 3].resize(2);
                        derivative_var_component_idx[d_num_species + 3].resize(2);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 3][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][0] = 0;
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 3][1] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][1] = 1;
                        
                        break;
                    }
                    case DIRECTION::Z_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable w.
                        derivative_var_data[d_num_species][0] = data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 2;
                        
                        derivative_var_data[d_num_species + 1].resize(0);
                        derivative_var_component_idx[d_num_species + 1].resize(0);
                        
                        derivative_var_data[d_num_species + 2].resize(1);
                        derivative_var_component_idx[d_num_species + 2].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 2][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 0;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 3].resize(2);
                        derivative_var_component_idx[d_num_species + 3].resize(2);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 3][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][0] = 0;
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 3][1] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][1] = 2;
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxVariablesForDerivative()\n"
                            << "There are only x-direction, y-direction and z-direction for three-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            case DIRECTION::Y_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species][0] = data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 1;
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 1][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 0;
                        
                        derivative_var_data[d_num_species + 2].resize(0);
                        derivative_var_component_idx[d_num_species + 2].resize(0);
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 3].resize(2);
                        derivative_var_component_idx[d_num_species + 3].resize(2);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 3][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][0] = 0;
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 3][1] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][1] = 1;
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species][0] = data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 0;
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 1][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 1;
                        
                        derivative_var_data[d_num_species + 2].resize(1);
                        derivative_var_component_idx[d_num_species + 2].resize(1);
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 2][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 2;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 3].resize(3);
                        derivative_var_component_idx[d_num_species + 3].resize(3);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 3][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][0] = 0;
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 3][1] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][1] = 1;
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 3][2] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][2] = 2;
                        
                        break;
                    }
                    case DIRECTION::Z_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(0);
                        derivative_var_component_idx[d_num_species].resize(0);
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 1][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 2;
                        
                        derivative_var_data[d_num_species + 2].resize(1);
                        derivative_var_component_idx[d_num_species + 2].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 2][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 1;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 3].resize(2);
                        derivative_var_component_idx[d_num_species + 3].resize(2);
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 3][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][0] = 1;
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 3][1] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][1] = 2;
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxVariablesForDerivative()\n"
                            << "There are only x-direction, y-direction and z-direction for three-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            case DIRECTION::Z_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable w.
                        derivative_var_data[d_num_species][0] = data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 2;
                        
                        derivative_var_data[d_num_species + 1].resize(0);
                        derivative_var_component_idx[d_num_species + 1].resize(0);
                        
                        derivative_var_data[d_num_species + 2].resize(1);
                        derivative_var_component_idx[d_num_species + 2].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 2][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 0;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 3].resize(2);
                        derivative_var_component_idx[d_num_species + 3].resize(2);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 3][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][0] = 0;
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 3][1] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][1] = 2;
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(0);
                        derivative_var_component_idx[d_num_species].resize(0);
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 1][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 2;
                        
                        derivative_var_data[d_num_species + 2].resize(1);
                        derivative_var_component_idx[d_num_species + 2].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 2][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 1;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 3].resize(2);
                        derivative_var_component_idx[d_num_species + 3].resize(2);
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 3][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][0] = 1;
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 3][1] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][1] = 2;
                        
                        break;
                    }
                    case DIRECTION::Z_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species][0] = data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 0;
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 1][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 1;
                        
                        derivative_var_data[d_num_species + 2].resize(1);
                        derivative_var_component_idx[d_num_species + 2].resize(1);
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 2][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 2;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 3].resize(3);
                        derivative_var_component_idx[d_num_species + 3].resize(3);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 3][0] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][0] = 0;
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 3][1] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][1] = 1;
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 3][2] = data_velocity;
                        derivative_var_component_idx[d_num_species + 3][2] = 2;
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxVariablesForDerivative()\n"
                            << "There are only x-direction, y-direction and z-direction for three-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxVariablesForDerivative()\n"
                    << "There are only x-direction, y-direction and z-direction for three-dimensional problem."
                    << std::endl);
            }
        }
    }
}


/*
 * Get the diffusivities in the diffusive flux.
 */
void
FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxDiffusivities(
    std::vector<std::vector<boost::shared_ptr<pdat::CellData<double> > > >& diffusivities_data,
    std::vector<std::vector<int> >& diffusivities_component_idx,
    const DIRECTION::TYPE& flux_direction,
    const DIRECTION::TYPE& derivative_direction)
{
    // Create empty box.
    const hier::Box empty_box(d_dim);
    
    if (d_flow_model.expired())
    {
        TBOX_ERROR(d_object_name
            << ": "
            << "The object is not setup yet!"
            << std::endl);
    }
    
    boost::shared_ptr<FlowModel> flow_model_tmp = d_flow_model.lock();
    const hier::Patch& patch = flow_model_tmp->getRegisteredPatch();
    
    diffusivities_data.resize(d_num_eqn);
    diffusivities_component_idx.resize(d_num_eqn);
    
    /*
     * Get the dimension of the interior box.
     */
    
    const hier::Box interior_box = patch.getBox();
    const hier::IntVector interior_dims = interior_box.numberCells();
    
    if (!d_derived_cell_data_computed)
    {
        // Get the cell data of volume fractions.
        boost::shared_ptr<pdat::CellData<double> > data_volume_fractions =
            flow_model_tmp->getCellData("VOLUME_FRACTIONS");
        
        // Get the cell data of mass fractions.
        boost::shared_ptr<pdat::CellData<double> > data_mass_fractions =
            flow_model_tmp->getCellData("MASS_FRACTIONS");
        
        // Get the cell data of velocity.
        boost::shared_ptr<pdat::CellData<double> > data_velocity =
            flow_model_tmp->getCellData("VELOCITY");
        
        // Get the cell data of pressure.
        boost::shared_ptr<pdat::CellData<double> > data_pressure =
            flow_model_tmp->getCellData("PRESSURE");
        
        // Get the cell data of species temperature.
        boost::shared_ptr<pdat::CellData<double> > data_species_temperatures =
            flow_model_tmp->getCellData("SPECIES_TEMPERATURES");
        
        /*
         * Get the number of ghost cells of velocity.
         */
        
        const hier::IntVector num_subghosts_velocity = data_velocity->getGhostCellWidth();
        
        /*
         * Get the dimensions of the ghost cell box of velocity.
         */
        
        const hier::Box subghost_box_velocity = data_velocity->getGhostBox();
        const hier::IntVector subghostcell_dims_velocity = subghost_box_velocity.numberCells();
        
        /*
         * Create cell data of shear viscosity and bulk viscosity.
         */
        
        d_data_shear_viscosity.reset(new pdat::CellData<double>(
            interior_box, 1, d_num_subghosts_shear_viscosity));
        
        d_data_bulk_viscosity.reset(new pdat::CellData<double>(
            interior_box, 1, d_num_subghosts_bulk_viscosity));
        
        /*
         * Get the pointers to the cell data of shear viscosity and bulk viscosity.
         */
        
        double* mu    = d_data_shear_viscosity->getPointer(0);
        double* mu_v  = d_data_bulk_viscosity->getPointer(0);
        
        // Compute the shear viscosity field.
        d_equation_of_shear_viscosity_mixing_rules->computeShearViscosity(
            d_data_shear_viscosity,
            data_pressure,
            data_species_temperatures,
            data_mass_fractions,
            data_volume_fractions,
            empty_box);
        
        // Compute the bulk viscosity field.
        d_equation_of_bulk_viscosity_mixing_rules->computeBulkViscosity(
            d_data_bulk_viscosity,
            data_pressure,
            data_species_temperatures,
            data_mass_fractions,
            data_volume_fractions,
            empty_box);
        
        if (d_dim == tbox::Dimension(1))
        {
            d_data_diffusivities.reset(new pdat::CellData<double>(
                interior_box,
                2,
                d_num_subghosts_diffusivities));
            
            // Get the pointer to cell data of velocity and diffusivities.
            double* u = data_velocity->getPointer(0);
            
            std::vector<double*> D_ptr;
            D_ptr.reserve(2);
            
            for (int i = 0; i < 2; i++)
            {
                D_ptr.push_back(d_data_diffusivities->getPointer(i));
            }
            
            /*
             * Compute the diffusivities.
             */
            for (int i = -d_num_subghosts_diffusivities[0];
                 i < interior_dims[0] + d_num_subghosts_diffusivities[0];
                 i++)
            {
                // Compute the linear indices.
                const int idx_diffusivities = i + d_num_subghosts_diffusivities[0];
                const int idx_shear_viscosity = i + d_num_subghosts_shear_viscosity[0];
                const int idx_bulk_viscosity = i + d_num_subghosts_bulk_viscosity[0];
                const int idx_velocity = i + num_subghosts_velocity[0];
                
                D_ptr[0][idx_diffusivities] =
                    -(double(4)/double(3)*mu[idx_shear_viscosity] + mu_v[idx_bulk_viscosity]);
                D_ptr[1][idx_diffusivities] =
                    -u[idx_velocity]*(double(4)/double(3)*mu[idx_shear_viscosity] + mu_v[idx_bulk_viscosity]);
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            d_data_diffusivities.reset(new pdat::CellData<double>(
                interior_box,
                9,
                d_num_subghosts_diffusivities));
            
            // Get the pointer to cell data of velocity and diffusivities.
            double* u = data_velocity->getPointer(0);
            double* v = data_velocity->getPointer(1);
            
            std::vector<double*> D_ptr;
            D_ptr.reserve(9);
            
            for (int i = 0; i < 9; i++)
            {
                D_ptr.push_back(d_data_diffusivities->getPointer(i));
            }
            
            /*
             * Compute the diffusivities.
             */
            for (int j = -d_num_subghosts_diffusivities[1];
                 j < interior_dims[1] + d_num_subghosts_diffusivities[1];
                 j++)
            {
                for (int i = -d_num_subghosts_diffusivities[0];
                     i < interior_dims[0] + d_num_subghosts_diffusivities[0];
                     i++)
                {
                    // Compute the linear indices.
                    const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                        (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0];
                    
                    const int idx_shear_viscosity = (i + d_num_subghosts_shear_viscosity[0]) +
                        (j + d_num_subghosts_shear_viscosity[1])*d_subghostcell_dims_shear_viscosity[0];
                    
                    const int idx_bulk_viscosity = (i + d_num_subghosts_bulk_viscosity[0]) +
                        (j + d_num_subghosts_bulk_viscosity[1])*d_subghostcell_dims_bulk_viscosity[0];
                    
                    const int idx_velocity = (i + num_subghosts_velocity[0]) +
                        (j + num_subghosts_velocity[1])*subghostcell_dims_velocity[0];
                    
                    D_ptr[0][idx_diffusivities] =
                        -(double(4)/double(3)*mu[idx_shear_viscosity] + mu_v[idx_bulk_viscosity]);
                    D_ptr[1][idx_diffusivities] =
                        double(2)/double(3)*mu[idx_shear_viscosity] - mu_v[idx_bulk_viscosity];
                    D_ptr[2][idx_diffusivities] =
                        -mu[idx_shear_viscosity];
                    D_ptr[3][idx_diffusivities] =
                        -u[idx_velocity]*(double(4)/double(3)*mu[idx_shear_viscosity] + mu_v[idx_bulk_viscosity]);
                    D_ptr[4][idx_diffusivities] =
                        -v[idx_velocity]*(double(4)/double(3)*mu[idx_shear_viscosity] + mu_v[idx_bulk_viscosity]);
                    D_ptr[5][idx_diffusivities] =
                        u[idx_velocity]*(double(2)/double(3)*mu[idx_shear_viscosity] - mu_v[idx_bulk_viscosity]);
                    D_ptr[6][idx_diffusivities] =
                        v[idx_velocity]*(double(2)/double(3)*mu[idx_shear_viscosity] - mu_v[idx_bulk_viscosity]);
                    D_ptr[7][idx_diffusivities] =
                        -u[idx_velocity]*mu[idx_shear_viscosity];
                    D_ptr[8][idx_diffusivities] =
                        -v[idx_velocity]*mu[idx_shear_viscosity];
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            d_data_diffusivities.reset(new pdat::CellData<double>(
                interior_box,
                12,
                d_num_subghosts_diffusivities));
            
            // Get the pointer to cell data of velocity and diffusivities.
            double* u = data_velocity->getPointer(0);
            double* v = data_velocity->getPointer(1);
            double* w = data_velocity->getPointer(2);
            
            std::vector<double*> D_ptr;
            D_ptr.reserve(12);
            
            for (int i = 0; i < 12; i++)
            {
                D_ptr.push_back(d_data_diffusivities->getPointer(i));
            }
            
            /*
             * Compute the diffusivities.
             */
            for (int k = -d_num_subghosts_diffusivities[2];
                 k < interior_dims[2] + d_num_subghosts_diffusivities[2];
                 k++)
            {
                for (int j = -d_num_subghosts_diffusivities[1];
                     j < interior_dims[1] + d_num_subghosts_diffusivities[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_diffusivities[0];
                         i < interior_dims[0] + d_num_subghosts_diffusivities[0];
                         i++)
                    {
                        const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                            (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0] +
                            (k + d_num_subghosts_diffusivities[2])*d_subghostcell_dims_diffusivities[0]*
                                d_subghostcell_dims_diffusivities[1];
                        
                        const int idx_shear_viscosity = (i + d_num_subghosts_shear_viscosity[0]) +
                            (j + d_num_subghosts_shear_viscosity[1])*d_subghostcell_dims_shear_viscosity[0] +
                            (k + d_num_subghosts_shear_viscosity[2])*d_subghostcell_dims_shear_viscosity[0]*
                                d_subghostcell_dims_shear_viscosity[1];
                        
                        const int idx_bulk_viscosity = (i + d_num_subghosts_bulk_viscosity[0]) +
                            (j + d_num_subghosts_bulk_viscosity[1])*d_subghostcell_dims_bulk_viscosity[0] +
                            (k + d_num_subghosts_bulk_viscosity[2])*d_subghostcell_dims_bulk_viscosity[0]*
                                d_subghostcell_dims_bulk_viscosity[1];
                        
                        const int idx_velocity = (i + num_subghosts_velocity[0]) +
                            (j + num_subghosts_velocity[1])*subghostcell_dims_velocity[0] +
                            (k + num_subghosts_velocity[2])*subghostcell_dims_velocity[0]*
                                subghostcell_dims_velocity[1];
                        
                        D_ptr[0][idx_diffusivities] =
                            -(double(4)/double(3)*mu[idx_shear_viscosity] + mu_v[idx_bulk_viscosity]);
                        D_ptr[1][idx_diffusivities] =
                            double(2)/double(3)*mu[idx_shear_viscosity] - mu_v[idx_bulk_viscosity];
                        D_ptr[2][idx_diffusivities] =
                            -mu[idx_shear_viscosity];
                        D_ptr[3][idx_diffusivities] =
                            -u[idx_velocity]*(double(4)/double(3)*mu[idx_shear_viscosity] + mu_v[idx_bulk_viscosity]);
                        D_ptr[4][idx_diffusivities] =
                            -v[idx_velocity]*(double(4)/double(3)*mu[idx_shear_viscosity] + mu_v[idx_bulk_viscosity]);
                        D_ptr[5][idx_diffusivities] =
                            -w[idx_velocity]*(double(4)/double(3)*mu[idx_shear_viscosity] + mu_v[idx_bulk_viscosity]);
                        D_ptr[6][idx_diffusivities] =
                            u[idx_velocity]*(double(2)/double(3)*mu[idx_shear_viscosity] - mu_v[idx_bulk_viscosity]);
                        D_ptr[7][idx_diffusivities] =
                            v[idx_velocity]*(double(2)/double(3)*mu[idx_shear_viscosity] - mu_v[idx_bulk_viscosity]);
                        D_ptr[8][idx_diffusivities] =
                            w[idx_velocity]*(double(2)/double(3)*mu[idx_shear_viscosity] - mu_v[idx_bulk_viscosity]);
                        D_ptr[9][idx_diffusivities] =
                            -u[idx_velocity]*mu[idx_shear_viscosity];
                        D_ptr[10][idx_diffusivities] =
                            -v[idx_velocity]*mu[idx_shear_viscosity];
                        D_ptr[11][idx_diffusivities] =
                            -w[idx_velocity]*mu[idx_shear_viscosity];
                    }
                }
            }
        }
        
        d_derived_cell_data_computed = true;
    }
    
    if (d_dim == tbox::Dimension(1))
    {
        switch (flux_direction)
        {
            case DIRECTION::X_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // -(4/3*mu + mu_v).
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] = 0;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // -u*(4/3*mu + mu_v).
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] = 1;
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxDiffusivities()\n"
                            << "There are only x-direction for one-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxDiffusivities()\n"
                    << "There are only x-direction for one-dimensional problem."
                    << std::endl);
            }
        }
    }
    else if (d_dim == tbox::Dimension(2))
    {
        switch (flux_direction)
        {
            case DIRECTION::X_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // -(4/3*mu + mu_v).
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] = 0;
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] = 2;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 2].resize(2);
                        diffusivities_component_idx[d_num_species + 2].resize(2);
                        
                        // -u*(4/3*mu + mu_v).
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] = 3;
                        
                        // -v*mu.
                        diffusivities_data[d_num_species + 2][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][1] = 8;
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // 2/3*mu - mu_v.
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] = 1;
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] = 2;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 2].resize(2);
                        diffusivities_component_idx[d_num_species + 2].resize(2);
                        
                        // -v*mu.
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] = 8;
                        
                        // u*(2/3*mu - mu_v).
                        diffusivities_data[d_num_species + 2][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][1] = 5;
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxDiffusivities()\n"
                            << "There are only x-direction and y-direction for two-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            case DIRECTION::Y_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] = 2;
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // 2/3*mu - mu_v.
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] = 1;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 2].resize(2);
                        diffusivities_component_idx[d_num_species + 2].resize(2);
                        
                        // v*(2/3*mu - mu_v).
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] = 6;
                        
                        // -u*mu.
                        diffusivities_data[d_num_species + 2][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][1] = 7;
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] = 2;
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // -(4/3*mu + mu_v).
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] = 0;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 2].resize(2);
                        diffusivities_component_idx[d_num_species + 2].resize(2);
                        
                        // -u*mu.
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] = 7;
                        
                        // -v*(4/3*mu + mu_v).
                        diffusivities_data[d_num_species + 2][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][1] = 4;
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxDiffusivities()\n"
                            << "There are only x-direction and y-direction for two-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxDiffusivities()\n"
                    << "There are only x-direction and y-direction for two-dimensional problem."
                    << std::endl);
            }
        }
    }
    else if (d_dim == tbox::Dimension(3))
    {
        switch (flux_direction)
        {
            case DIRECTION::X_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // -(4/3*mu + mu_v).
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] = 0;
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] = 2;
                        
                        diffusivities_data[d_num_species + 2].resize(1);
                        diffusivities_component_idx[d_num_species + 2].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] = 2;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 3].resize(3);
                        diffusivities_component_idx[d_num_species + 3].resize(3);
                        
                        // -u*(4/3*mu + mu_v).
                        diffusivities_data[d_num_species + 3][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][0] = 3;
                        
                        // -v*mu.
                        diffusivities_data[d_num_species + 3][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][1] = 10;
                        
                        // -w*mu.
                        diffusivities_data[d_num_species + 3][2] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][2] = 11;
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // 2/3*mu - mu_v.
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] = 1;
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] = 2;
                        
                        diffusivities_data[d_num_species + 2].resize(0);
                        diffusivities_component_idx[d_num_species + 2].resize(0);
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 3].resize(2);
                        diffusivities_component_idx[d_num_species + 3].resize(2);
                        
                        // -v*mu.
                        diffusivities_data[d_num_species + 3][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][0] = 10;
                        
                        // u*(2/3*mu - mu_v).
                        diffusivities_data[d_num_species + 3][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][1] = 6;
                        
                        break;
                    }
                    case DIRECTION::Z_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // 2/3*mu - mu_v.
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] = 1;
                        
                        diffusivities_data[d_num_species + 1].resize(0);
                        diffusivities_component_idx[d_num_species + 1].resize(0);
                        
                        diffusivities_data[d_num_species + 2].resize(1);
                        diffusivities_component_idx[d_num_species + 2].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] = 2;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 3].resize(2);
                        diffusivities_component_idx[d_num_species + 3].resize(2);
                        
                        // -w*mu.
                        diffusivities_data[d_num_species + 3][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][0] = 11;
                        
                        // u*(2/3*mu - mu_v).
                        diffusivities_data[d_num_species + 3][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][1] = 6;
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxDiffusivities()\n"
                            << "There are only x-direction, y-direction and z-direction for three-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            case DIRECTION::Y_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] = 2;
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // 2/3*mu - mu_v.
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] = 1;
                        
                        diffusivities_data[d_num_species + 2].resize(0);
                        diffusivities_component_idx[d_num_species + 2].resize(0);
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 3].resize(2);
                        diffusivities_component_idx[d_num_species + 3].resize(2);
                        
                        // v*(2/3*mu - mu_v).
                        diffusivities_data[d_num_species + 3][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][0] = 7;
                        
                        // -u*mu.
                        diffusivities_data[d_num_species + 3][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][1] = 9;
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] = 2;
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // -(4/3*mu + mu_v).
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] = 0;
                        
                        diffusivities_data[d_num_species + 2].resize(1);
                        diffusivities_component_idx[d_num_species + 2].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] = 2;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 3].resize(3);
                        diffusivities_component_idx[d_num_species + 3].resize(3);
                        
                        // -u*mu.
                        diffusivities_data[d_num_species + 3][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][0] = 9;
                        
                        // -v*(4/3*mu + mu_v).
                        diffusivities_data[d_num_species + 3][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][1] = 4;
                        
                        // -w*mu.
                        diffusivities_data[d_num_species + 3][2] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][2] = 11;
                        
                        break;
                    }
                    case DIRECTION::Z_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(0);
                        diffusivities_component_idx[d_num_species].resize(0);
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // 2/3*(mu - mu_v).
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] = 1;
                        
                        diffusivities_data[d_num_species + 2].resize(1);
                        diffusivities_component_idx[d_num_species + 2].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] = 2;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 3].resize(2);
                        diffusivities_component_idx[d_num_species + 3].resize(2);
                        
                        // -w*u.
                        diffusivities_data[d_num_species + 3][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][0] = 11;
                        
                        // v*(2/3*mu - mu_v).
                        diffusivities_data[d_num_species + 3][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][1] = 7;
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxDiffusivities()\n"
                            << "There are only x-direction, y-direction and z-direction for three-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            case DIRECTION::Z_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] = 2;
                        
                        diffusivities_data[d_num_species + 1].resize(0);
                        diffusivities_component_idx[d_num_species + 1].resize(0);
                        
                        diffusivities_data[d_num_species + 2].resize(1);
                        diffusivities_component_idx[d_num_species + 2].resize(1);
                        
                        // 2/3*mu - mu_v.
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] = 1;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 3].resize(2);
                        diffusivities_component_idx[d_num_species + 3].resize(2);
                        
                        // w*(2/3*mu - mu_v).
                        diffusivities_data[d_num_species + 3][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][0] = 8;
                        
                        // -u*mu.
                        diffusivities_data[d_num_species + 3][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][1] = 9;
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(0);
                        diffusivities_component_idx[d_num_species].resize(0);
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] = 2;
                        
                        diffusivities_data[d_num_species + 2].resize(1);
                        diffusivities_component_idx[d_num_species + 2].resize(1);
                        
                        // 2/3*mu - mu_v.
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] = 1;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 3].resize(2);
                        diffusivities_component_idx[d_num_species + 3].resize(2);
                        
                        // w*(2/3*mu - mu_v).
                        diffusivities_data[d_num_species + 3][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][0] = 8;
                        
                        // -v*mu.
                        diffusivities_data[d_num_species + 3][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][1] = 10;
                        
                        break;
                    }
                    case DIRECTION::Z_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] = 2;
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] = 2;
                        
                        diffusivities_data[d_num_species + 2].resize(1);
                        diffusivities_component_idx[d_num_species + 2].resize(1);
                        
                        // -(4/3*mu + mu_v).
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] = 0;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 3].resize(3);
                        diffusivities_component_idx[d_num_species + 3].resize(3);
                        
                        // -u*mu.
                        diffusivities_data[d_num_species + 3][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][0] = 9;
                        
                        // -v*mu.
                        diffusivities_data[d_num_species + 3][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][1] = 10;
                        
                        // -w*(4/3*mu + mu_v).
                        diffusivities_data[d_num_species + 3][2] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][2] = 5;
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxDiffusivities()\n"
                            << "There are only x-direction, y-direction and z-direction for three-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelDiffusiveFluxUtilitiesFiveEqnAllaire::getDiffusiveFluxDiffusivities()\n"
                    << "There are only x-direction, y-direction and z-direction for three-dimensional problem."
                    << std::endl);
            }
        }
    }
}
