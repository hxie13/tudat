#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include "Tudat/Astrodynamics/BasicAstrodynamics/orbitalElementConversions.h"
#include "Tudat/Basics/testMacros.h"

#include "Tudat/Astrodynamics/BasicAstrodynamics/accelerationModel.h"
#include "Tudat/External/SpiceInterface/spiceInterface.h"

#include "Tudat/Astrodynamics/ElectroMagnetism/panelledRadiationPressure.h"
#include "Tudat/Astrodynamics/Ephemerides/keplerEphemeris.h"
#include "Tudat/Astrodynamics/Ephemerides/simpleRotationalEphemeris.h"
#include "Tudat/Astrodynamics/Ephemerides/constantRotationalEphemeris.h"
#include "Tudat/SimulationSetup/EnvironmentSetup/createBodies.h"
#include "Tudat/SimulationSetup/EnvironmentSetup/body.h"
#include "Tudat/SimulationSetup/EnvironmentSetup/defaultBodies.h"
#include "Tudat/SimulationSetup/PropagationSetup/createAccelerationModels.h"

namespace tudat
{

using namespace tudat::basic_astrodynamics;
using namespace tudat::simulation_setup;
using namespace tudat::ephemerides;
using namespace tudat::basic_astrodynamics;
using namespace tudat::electro_magnetism;

namespace unit_tests
{

BOOST_AUTO_TEST_SUITE( test_PanelledRadiationPressure )

BOOST_AUTO_TEST_CASE( testSimpleGeometryPanelledRadiationPressure )
{
    //Load spice kernels.
    spice_interface::loadStandardSpiceKernels( );

    // Create bodies needed in simulation
    double initialEphemerisTime = -86400.0;
    double finalEphemerisTime = 1.1 * 365.25 * 86400.0;
    std::vector< std::string > bodyNames;
    bodyNames.push_back( "Sun" );
    NamedBodyMap bodyMap = createBodies(
                getDefaultBodySettings( bodyNames,initialEphemerisTime, finalEphemerisTime ) );

    // Create vehicle
    double vehicleMass = 2500.0;
    bodyMap[ "Vehicle" ] = std::make_shared< Body >( );
    bodyMap[ "Vehicle" ]->setConstantBodyMass( vehicleMass );

    // Put vehicle on circular orbit around Sun
    Eigen::Vector6d initialStateInKeplerianElements = Eigen::Vector6d::Zero( );
    initialStateInKeplerianElements[ 0 ] = physical_constants::ASTRONOMICAL_UNIT;
    bodyMap[ "Vehicle" ]->setEphemeris(
                std::make_shared< KeplerEphemeris >(
                    initialStateInKeplerianElements, 0.0, spice_interface::getBodyGravitationalParameter( "Sun" ),
                    "Sun", "ECLIPJ2000", 1 ) );
    Eigen::Vector7d rotationalStateVehicle;
    rotationalStateVehicle.segment( 0, 4 ) = linear_algebra::convertQuaternionToVectorFormat( Eigen::Quaterniond( Eigen::Matrix3d::Identity() ));
    rotationalStateVehicle.segment( 4, 3 ) = Eigen::Vector3d::Zero();
//    bodyMap[ "Vehicle" ]->setRotationalEphemeris(
//                std::make_shared< ConstantRotationalEphemeris >(
//                    Eigen::Quaterniond( Eigen::Matrix3d::Identity( ) ), "ECLIPJ2000", "VehicleFixed" ) );
    bodyMap[ "Vehicle" ]->setRotationalEphemeris(
                std::make_shared< ConstantRotationalEphemeris >(
                    rotationalStateVehicle, "ECLIPJ2000", "VehicleFixed" ) );

    setGlobalFrameBodyEphemerides( bodyMap, "SSB", "ECLIPJ2000" );

    for( unsigned int test = 0; test < 2; test++ )
    {
        // Create radiation pressure properties
        std::vector< double > areas;
        areas.push_back( 2.532 );
        areas.push_back( 3.254 );
        areas.push_back( 8.654 );
        areas.push_back( 1.346 );
        areas.push_back( 2.454 );
        areas.push_back( 5.345 );

        std::vector< double > emissivities;
        if( test == 0 )
        {
            emissivities.push_back( 0.0 );
            emissivities.push_back( 1.0 );
            emissivities.push_back( 0.0 );
            emissivities.push_back( 1.0 );
        }
        else if( test == 1 )
        {
            emissivities.push_back( 0.2 );
            emissivities.push_back( 0.3 );
            emissivities.push_back( 0.4 );
            emissivities.push_back( 0.5 );
        }
        emissivities.push_back( 0.5 );
        emissivities.push_back( 0.2 );

        std::vector< double > diffuseReflectionCoefficients;
        if( test == 0 )
        {
            diffuseReflectionCoefficients.push_back( 0.0 );
            diffuseReflectionCoefficients.push_back( 0.0 );
            diffuseReflectionCoefficients.push_back( 0.0 );
            diffuseReflectionCoefficients.push_back( 0.0 );
            diffuseReflectionCoefficients.push_back( 0.0 );
            diffuseReflectionCoefficients.push_back( 0.0 );
        }
        else
        {
            diffuseReflectionCoefficients.push_back( 0.6 );
            diffuseReflectionCoefficients.push_back( 0.5 );
            diffuseReflectionCoefficients.push_back( 0.4 );
            diffuseReflectionCoefficients.push_back( 0.3 );
            diffuseReflectionCoefficients.push_back( 0.2 );
            diffuseReflectionCoefficients.push_back( 0.1 );
        }

        if( test == 2 )
        {
            bodyMap[ "Vehicle" ]->setRotationalEphemeris(
                        std::make_shared< tudat::ephemerides::SimpleRotationalEphemeris >(
                            0.2, 0.4, -0.2, 1.0E-5, 0.0, "ECLIPJ2000", "VehicleFixed" ) );
        }

        std::vector< Eigen::Vector3d > panelSurfaceNormals;
        panelSurfaceNormals.push_back( -Eigen::Vector3d::UnitX( ) );
        panelSurfaceNormals.push_back( -Eigen::Vector3d::UnitY( ) );
        panelSurfaceNormals.push_back( Eigen::Vector3d::UnitX( ) );
        panelSurfaceNormals.push_back( Eigen::Vector3d::UnitY( ) );
        panelSurfaceNormals.push_back( Eigen::Vector3d::UnitZ( ) );
        panelSurfaceNormals.push_back( -Eigen::Vector3d::UnitZ( ) );

        std::shared_ptr< PanelledRadiationPressureInterfaceSettings > radiationPressureInterfaceSettings =
                std::make_shared< PanelledRadiationPressureInterfaceSettings >(
                    "Sun", emissivities, areas, diffuseReflectionCoefficients, panelSurfaceNormals );
        std::shared_ptr< PanelledRadiationPressureInterface > radiationPressureInterface =
                std::dynamic_pointer_cast< PanelledRadiationPressureInterface >(
                    createRadiationPressureInterface( radiationPressureInterfaceSettings, "Vehicle", bodyMap ) );
        bodyMap[ "Vehicle" ]->setRadiationPressureInterface( "Sun", radiationPressureInterface );

        // Define accelerations
        SelectedAccelerationMap accelerationMap;
        std::map< std::string, std::vector< std::shared_ptr< AccelerationSettings > > > accelerationsOnVehicle;
        accelerationsOnVehicle[ "Sun" ].push_back( std::make_shared< AccelerationSettings >(
                                                       panelled_radiation_pressure_acceleration ) );
        accelerationMap[ "Vehicle" ] = accelerationsOnVehicle;
        std::map< std::string, std::string > centralBodyMap;
        centralBodyMap[ "Vehicle" ] = "Sun";
        AccelerationMap accelerationModelMap = createAccelerationModelsMap( bodyMap, accelerationMap, centralBodyMap );
        std::shared_ptr< AccelerationModel< Eigen::Vector3d > > accelerationModel =
                accelerationModelMap.at( "Vehicle" ).at( "Sun" ).at( 0 );

        // Compute spacecraft orbital period, and compute test times
        double orbitalPeriod =
                2.0 * mathematical_constants::PI * std::sqrt(
                    std::pow( physical_constants::ASTRONOMICAL_UNIT, 3.0 ) /
                    spice_interface::getBodyGravitationalParameter( "Sun" ) );
        std::vector< double > testTimes = { 0.0, orbitalPeriod / 4.0, orbitalPeriod / 2.0, 3.0 * orbitalPeriod / 4.0 };

        // Compute panelled radiation pressure for various relative Sun positions
        Eigen::Vector3d calculatedAcceleration, expectedAcceleration;
        Eigen::Vector3d sunCenteredVehiclePosition;
        std::shared_ptr< Ephemeris > vehicleEphemeris = bodyMap[ "Vehicle" ]->getEphemeris( );
        for( unsigned int i = 0; i < testTimes.size( ); i++ )
        {
            // Update environment and acceleration
            bodyMap[ "Sun" ]->setStateFromEphemeris( testTimes[ i ] );
            bodyMap[ "Vehicle" ]->setStateFromEphemeris( testTimes[ i ] );
            bodyMap[ "Vehicle" ]->setCurrentRotationToLocalFrameFromEphemeris( testTimes[ i ] );
            radiationPressureInterface->updateInterface( testTimes[ i ] );
            accelerationModel->updateMembers( testTimes[ i ] );

            // Retrievce acceleration
            calculatedAcceleration = accelerationModel->getAcceleration( );

            // Manually compute acceleration
            double radiationPressure = radiationPressureInterface->getCurrentRadiationPressure( );
            sunCenteredVehiclePosition = vehicleEphemeris->getCartesianState( testTimes[ i ] ).segment( 0, 3 );
            if( test == 0 )
            {
                if( i == 0 || i == 2 )
                {
                    expectedAcceleration = radiationPressure / vehicleMass  *
                            radiationPressureInterface->getArea( i ) * sunCenteredVehiclePosition.normalized( );
                }
                else if( i == 1 || i == 3 )
                {
                    expectedAcceleration = -radiationPressure / vehicleMass * (
                                2.0 * radiationPressureInterface->getArea( i ) *
                                radiationPressureInterface->getCurrentSurfaceNormal( i ) );
                }
            }
            else
            {
                expectedAcceleration = radiationPressure / vehicleMass *
                        ( 1.0 + radiationPressureInterface->getEmissivity( i ) + 2.0 * diffuseReflectionCoefficients.at( i ) / 3.0 ) *
                        radiationPressureInterface->getArea( i ) * sunCenteredVehiclePosition.normalized( );
            }

            if( test == 0 || test == 1 )
            {
                // Check computed acceleration
                for( unsigned int j = 0; j < 3; j++ )
                {
                    BOOST_CHECK_SMALL( std::fabs( calculatedAcceleration( j ) - expectedAcceleration( j ) ), 2.0E-23 );
                }
            }
            else
            {
                std::shared_ptr< PanelledRadiationPressureAcceleration > panelledRadiationPressureAcceleration =
                        std::dynamic_pointer_cast< PanelledRadiationPressureAcceleration >( accelerationModel );
                Eigen::Quaterniond currentRotationToInertialFrame =
                        bodyMap[ "Vehicle" ]->getRotationalEphemeris( )->getRotationToBaseFrame( testTimes.at( i ) );
                for( unsigned int j = 0; j < panelSurfaceNormals.size( ); j++ )
                {
                   Eigen::Vector3d currentPanelNormal =
                           panelledRadiationPressureAcceleration->getCurrentPanelSurfaceNormalInPropagationFrame( j );
                   Eigen::Vector3d expectedPanelNormal =
                           currentRotationToInertialFrame * panelSurfaceNormals.at( j );
                   for( unsigned int k = 0; k < 3; k++ )
                   {
                       BOOST_CHECK_SMALL( std::fabs( currentPanelNormal( k ) - expectedPanelNormal( k ) ), 1.0E-15 );
                   }
                }
            }
        }
    }
}


BOOST_AUTO_TEST_CASE( testPanelledRadiationPressureMontenbruckData )
{

    // Benchmark data is obtained from Montenbruck, O., 2017.
    // Semi-analytical solar radiation pressure modeling for QZS-1 orbit-normal and yaw-steering attitude.
    // Advances in Space Research, 59(8), pp.2088-2100..

    //Load spice kernels.
    spice_interface::loadStandardSpiceKernels( );

    // Create bodies needed in simulation
    double initialEphemerisTime = -86400.0;
    double finalEphemerisTime = 1.1 * 365.25 * 86400.0;
    std::vector< std::string > bodyNames;
    bodyNames.push_back( "Sun" );
    NamedBodyMap bodyMap = createBodies(
                getDefaultBodySettings( bodyNames,initialEphemerisTime, finalEphemerisTime ) );

    // Create vehicle
    double vehicleMass = 2000.0;
    bodyMap[ "Vehicle" ] = std::make_shared< Body >( );
    bodyMap[ "Vehicle" ]->setConstantBodyMass( vehicleMass );

    // Put vehicle on circular orbit around Sun
    Eigen::Vector6d initialStateInKeplerianElements = Eigen::Vector6d::Zero( );
    initialStateInKeplerianElements[ 0 ] = physical_constants::ASTRONOMICAL_UNIT;
    bodyMap[ "Vehicle" ]->setEphemeris(
                std::make_shared< KeplerEphemeris >(
                    initialStateInKeplerianElements, 0.0, spice_interface::getBodyGravitationalParameter( "Sun" ),
                    "Sun", "ECLIPJ2000", 1 ) );

    Eigen::Vector7d rotationalStateVehicle;
    rotationalStateVehicle.segment( 0, 4 ) = linear_algebra::convertQuaternionToVectorFormat( Eigen::Quaterniond( Eigen::Matrix3d::Identity() ));
    rotationalStateVehicle.segment( 4, 3 ) = Eigen::Vector3d::Zero();
    bodyMap[ "Vehicle" ]->setRotationalEphemeris(
                std::make_shared< ConstantRotationalEphemeris >(
                    rotationalStateVehicle, "ECLIPJ2000", "VehicleFixed" ) );

    setGlobalFrameBodyEphemerides( bodyMap, "SSB", "ECLIPJ2000" );


    // Create radiation pressure properties
    std::vector< double > areas;
//    areas.push_back( 2.0 );
//    areas.push_back( 4.0 );
//    areas.push_back( 6.0 );
    areas.push_back( 9.9 );
    areas.push_back( 2.3 );
//    areas.push_back( 9.9 );
//    areas.push_back( 2.3 );
//    areas.push_back( 4.6 );
//    areas.push_back( 5.3 );
//    areas.push_back( 2.7 );
//    areas.push_back( 5.8 );
//    areas.push_back( 4.1 );
//    areas.push_back( 2.7 );

    std::vector< double > emissivities;
//    emissivities.push_back( 0.0 );
//    emissivities.push_back( 0.1 );
//    emissivities.push_back( 0.0 );
    emissivities.push_back( 0.0 );
    emissivities.push_back( 0.1 );
//    emissivities.push_back( 0.0 );
//    emissivities.push_back( 0.1 );
//    emissivities.push_back( 0.0 );
//    emissivities.push_back( 0.94 );
//    emissivities.push_back( 0.1 );
//    emissivities.push_back( 0.0 );
//    emissivities.push_back( 0.94 );
//    emissivities.push_back( 0.1 );
//    emissivities.push_back( 0.21 );

    std::vector< double > diffuseReflectionCoefficients;
//    diffuseReflectionCoefficients.push_back( 0.06 );
//    diffuseReflectionCoefficients.push_back( 0.46 );
//    diffuseReflectionCoefficients.push_back( 0.06 );
    diffuseReflectionCoefficients.push_back( 0.06 );
    diffuseReflectionCoefficients.push_back( 0.46 );
//    diffuseReflectionCoefficients.push_back( 0.06 );
//    diffuseReflectionCoefficients.push_back( 0.46 );
//    diffuseReflectionCoefficients.push_back( 0.06 );
//    diffuseReflectionCoefficients.push_back( 0.06 );
//    diffuseReflectionCoefficients.push_back( 0.46 );
//    diffuseReflectionCoefficients.push_back( 0.06 );
//    diffuseReflectionCoefficients.push_back( 0.06 );
//    diffuseReflectionCoefficients.push_back( 0.46 );
//    diffuseReflectionCoefficients.push_back( 0.21 );

    Eigen::Vector3d orientationSolarPanel;

    std::vector< Eigen::Vector3d > panelSurfaceNormals;
//    panelSurfaceNormals.push_back( Eigen::Vector3d::UnitZ( ) );
//    panelSurfaceNormals.push_back( Eigen::Vector3d::UnitZ( ) );
//    panelSurfaceNormals.push_back( - Eigen::Vector3d::UnitZ( ) );
    panelSurfaceNormals.push_back( Eigen::Vector3d::UnitX( ) );
    panelSurfaceNormals.push_back( Eigen::Vector3d::UnitX( ) );
//    panelSurfaceNormals.push_back( - Eigen::Vector3d::UnitX( ) );
//    panelSurfaceNormals.push_back( - Eigen::Vector3d::UnitX( ) );
//    panelSurfaceNormals.push_back( Eigen::Vector3d::UnitY( ) );
//    panelSurfaceNormals.push_back( Eigen::Vector3d::UnitY( ) );
//    panelSurfaceNormals.push_back( Eigen::Vector3d::UnitY( ) );
//    panelSurfaceNormals.push_back( - Eigen::Vector3d::UnitY( ) );
//    panelSurfaceNormals.push_back( - Eigen::Vector3d::UnitY( ) );
//    panelSurfaceNormals.push_back( - Eigen::Vector3d::UnitY( ) );
//    panelSurfaceNormals.push_back( orientationSolarPanel );


    std::vector< double > expectedAccelerationsDueToEmissivity;
    expectedAccelerationsDueToEmissivity.push_back( 4.6 );
    expectedAccelerationsDueToEmissivity.push_back( 8.2 );
    expectedAccelerationsDueToEmissivity.push_back( 13.7 );
    expectedAccelerationsDueToEmissivity.push_back( 22.6 );
    expectedAccelerationsDueToEmissivity.push_back( 4.7 );
    expectedAccelerationsDueToEmissivity.push_back( 22.6 );
    expectedAccelerationsDueToEmissivity.push_back( 4.7 );
    expectedAccelerationsDueToEmissivity.push_back( 10.5 );
    expectedAccelerationsDueToEmissivity.push_back( 0.7 );
    expectedAccelerationsDueToEmissivity.push_back( 5.5 );
    expectedAccelerationsDueToEmissivity.push_back( 13.2 );
    expectedAccelerationsDueToEmissivity.push_back( 0.6 );
    expectedAccelerationsDueToEmissivity.push_back( 5.5 );
    expectedAccelerationsDueToEmissivity.push_back( 72.1 );

    std::vector< double > expectedAccelerationsDueToDiffuseReflectivities;
    expectedAccelerationsDueToDiffuseReflectivities.push_back( 0.0 );
    expectedAccelerationsDueToDiffuseReflectivities.push_back( 0.9 );
    expectedAccelerationsDueToDiffuseReflectivities.push_back( 0.0 );
    expectedAccelerationsDueToDiffuseReflectivities.push_back( 0.0 );
    expectedAccelerationsDueToDiffuseReflectivities.push_back( 0.5 );
    expectedAccelerationsDueToDiffuseReflectivities.push_back( 0.0 );
    expectedAccelerationsDueToDiffuseReflectivities.push_back( 0.5 );
    expectedAccelerationsDueToDiffuseReflectivities.push_back( 0.0 );
    expectedAccelerationsDueToDiffuseReflectivities.push_back( 11.4 );
    expectedAccelerationsDueToDiffuseReflectivities.push_back( 0.6 );
    expectedAccelerationsDueToDiffuseReflectivities.push_back( 0.0 );
    expectedAccelerationsDueToDiffuseReflectivities.push_back( 8.8 );
    expectedAccelerationsDueToDiffuseReflectivities.push_back( 0.6 );
    expectedAccelerationsDueToDiffuseReflectivities.push_back( 19.2 );


    std::shared_ptr< PanelledRadiationPressureInterfaceSettings > radiationPressureInterfaceSettings =
            std::make_shared< PanelledRadiationPressureInterfaceSettings >(
                "Sun", emissivities, areas, diffuseReflectionCoefficients, panelSurfaceNormals );

    std::shared_ptr< PanelledRadiationPressureInterface > radiationPressureInterface =
            std::dynamic_pointer_cast< PanelledRadiationPressureInterface >(
                createRadiationPressureInterface( radiationPressureInterfaceSettings, "Vehicle", bodyMap ) );

    bodyMap[ "Vehicle" ]->setRadiationPressureInterface( "Sun", radiationPressureInterface );



    // Define accelerations
    SelectedAccelerationMap accelerationMap;
    std::map< std::string, std::vector< std::shared_ptr< AccelerationSettings > > > accelerationsOnVehicle;
    accelerationsOnVehicle[ "Sun" ].push_back( std::make_shared< AccelerationSettings >(
                                                   panelled_radiation_pressure_acceleration ) );

    accelerationMap[ "Vehicle" ] = accelerationsOnVehicle;

    std::map< std::string, std::string > centralBodyMap;
    centralBodyMap[ "Vehicle" ] = "Sun";
    AccelerationMap accelerationModelMap = createAccelerationModelsMap( bodyMap, accelerationMap, centralBodyMap );
    std::shared_ptr< AccelerationModel< Eigen::Vector3d > > accelerationModel =
            accelerationModelMap.at( "Vehicle" ).at( "Sun" ).at( 0 );

    // Compute spacecraft orbital period, and compute test times
    double orbitalPeriod =
            2.0 * mathematical_constants::PI * std::sqrt(
                std::pow( physical_constants::ASTRONOMICAL_UNIT, 3.0 ) /
                spice_interface::getBodyGravitationalParameter( "Sun" ) );
    std::vector< double > testTimes = { 0.0, orbitalPeriod / 4.0, orbitalPeriod / 2.0, 3.0 * orbitalPeriod / 4.0 };


//    // Manually compute acceleration
//    double radiationPressure = radiationPressureInterface->getCurrentRadiationPressure( );

//    std::cout << "radiation pressure: " << radiationPressure << "\n\n";

    // Compute panelled radiation pressure for various relative Sun positions
    Eigen::Vector3d calculatedAcceleration, expectedAcceleration;
    Eigen::Vector3d sunCenteredVehiclePosition;
    std::shared_ptr< Ephemeris > vehicleEphemeris = bodyMap[ "Vehicle" ]->getEphemeris( );
//    for( unsigned int i = 0; i < testTimes.size( ); i++ )
//    {
        // Update environment and acceleration
        bodyMap[ "Sun" ]->setStateFromEphemeris( initialEphemerisTime ); // testTimes[ i ] );
        bodyMap[ "Vehicle" ]->setStateFromEphemeris( initialEphemerisTime ); //testTimes[ i ] );
        bodyMap[ "Vehicle" ]->setCurrentRotationToLocalFrameFromEphemeris( initialEphemerisTime ); //testTimes[ i ] );
        radiationPressureInterface->updateInterface( initialEphemerisTime ); //testTimes[ i ] );
        accelerationModel->updateMembers( initialEphemerisTime ); //testTimes[ i ] );

        // Retrievce acceleration
        calculatedAcceleration = accelerationModel->getAcceleration( );

        std::cout << "calculated accelerations: " << calculatedAcceleration << "\n\n";

        // Benchmark solar radiation pressure
        double solarFluxAt1AU = 1367.0; // W.m^(-2)
        double expectedSolarRadiationPressure = solarFluxAt1AU / physical_constants::SPEED_OF_LIGHT;

        std::cout << "expected radiation pressure: " << expectedSolarRadiationPressure << "\n\n";

        double radiationPressure = radiationPressureInterface->getCurrentRadiationPressure( );
        sunCenteredVehiclePosition = vehicleEphemeris->getCartesianState( initialEphemerisTime ).segment( 0, 3 ); //testTimes[ i ] ).segment( 0, 3 );
        std::cout << "radiation pressure: " << radiationPressure << "\n\n";

        std::cout << "test: " << 9.9 / 2000 * expectedSolarRadiationPressure * 1.0 << "\n\n";

//        if( test == 0 )
//        {
//            if( i == 0 || i == 2 )
//            {
//                expectedAcceleration = radiationPressure / vehicleMass  *
//                        radiationPressureInterface->getArea( i ) * sunCenteredVehiclePosition.normalized( );
//            }
//            else if( i == 1 || i == 3 )
//            {
//                expectedAcceleration = -radiationPressure / vehicleMass * (
//                            2.0 * radiationPressureInterface->getArea( i ) *
//                            radiationPressureInterface->getCurrentSurfaceNormal( i ) );
//            }
//        }
//        else
//        {
//            expectedAcceleration = radiationPressure / vehicleMass *
//                    ( 1.0 + radiationPressureInterface->getEmissivity( i ) + 2.0 * diffuseReflectionCoefficients.at( i ) / 3.0 ) *
//                    radiationPressureInterface->getArea( i ) * sunCenteredVehiclePosition.normalized( );
//        }

//        if( test == 0 || test == 1 )
//        {
//            // Check computed acceleration
//            for( unsigned int j = 0; j < 3; j++ )
//            {
//                BOOST_CHECK_SMALL( std::fabs( calculatedAcceleration( j ) - expectedAcceleration( j ) ), 2.0E-23 );
//            }
//        }
//        else
//        {
//            std::shared_ptr< PanelledRadiationPressureAcceleration > panelledRadiationPressureAcceleration =
//                    std::dynamic_pointer_cast< PanelledRadiationPressureAcceleration >( accelerationModel );
//            Eigen::Quaterniond currentRotationToInertialFrame =
//                    bodyMap[ "Vehicle" ]->getRotationalEphemeris( )->getRotationToBaseFrame( testTimes.at( i ) );
//            for( unsigned int j = 0; j < panelSurfaceNormals.size( ); j++ )
//            {
//               Eigen::Vector3d currentPanelNormal =
//                       panelledRadiationPressureAcceleration->getCurrentPanelSurfaceNormalInPropagationFrame( j );
//               Eigen::Vector3d expectedPanelNormal =
//                       currentRotationToInertialFrame * panelSurfaceNormals.at( j );
//               for( unsigned int k = 0; k < 3; k++ )
//               {
//                   BOOST_CHECK_SMALL( std::fabs( currentPanelNormal( k ) - expectedPanelNormal( k ) ), 1.0E-15 );
//               }
//            }
//        }


}



BOOST_AUTO_TEST_SUITE_END( )


}

}