// transformation_generator
//

#include <transformation_generator/transformation_generator.h>

const double BEADS_SEPERATION_VALUE = 0.0254;
const double PI = 3.14159265359;

// Constructor
// TransformationGenerator::TransformationGenerator(ros::NodeHandle* nodehandle) : nh_(*nodehandle)
// {
//  // initializePublishers();
// }

TransformationGenerator::TransformationGenerator()
{
    // initializePublishers();
}
// given randomly generated transformation matrix, this function can give a list of
// beads position
void TransformationGenerator::getBeadsPosition(int beads_number, int row_num, int col_num, 
        transformation_generator::ListOfPoints& list_of_beads_pos,
        const Eigen::Affine3d& trans_mat)
{
    list_of_beads_pos.points.clear(); // clear vector;
    geometry_msgs::Point bead_position;

    Eigen::Vector3d Ob;
    
    // Eigen::Vector3d Oe = trans_mat.translation();
    Eigen::Vector3d beads_in_sensor_frame;

    for (int i = 0; i < row_num; ++i)
    {
        Ob(0)= BEADS_SEPERATION_VALUE * i; // start from first row
        for (int j = 0; j < col_num; ++j)
        {
            Ob(1)= BEADS_SEPERATION_VALUE * j; // start from first colu
            Ob(2)= 0; // always zero, because beads z coordinate in body frame is zero
            beads_in_sensor_frame = trans_mat * Ob;
            bead_position.x = beads_in_sensor_frame(0);
            bead_position.y = beads_in_sensor_frame(1);
            bead_position.z = beads_in_sensor_frame(2);
            list_of_beads_pos.points.push_back(bead_position);
        }
    }
    // int npts = list_of_beads_pos.points.size();
    // beads_pos_pub_.publish(list_of_beads_pos);
}

// void TransformationGenerator::initializePublishers()
// {
//  ROS_INFO("Initializing Publishers");
//  beads_pos_pub_ = nh_.advertise<transformation_generator::ListOfPoints>("beads_random_position", 1, true);
//  //add more publishers, as needed
//  // note: COULD make minimal_publisher_ a public member function, if want to use it within "main()"
// }

Eigen::Affine3d TransformationGenerator::getNewTransformationMatrix(const Eigen::Affine3d& old_trans_mat, 
                                                    const std::string flag, double delta_time)
{
    Eigen::Affine3d new_trans_mat;

    Eigen::Affine3d expo_mat = getExpoMatrix(old_trans_mat, flag, delta_time);

    new_trans_mat = expo_mat * old_trans_mat;

    // ROS_INFO_STREAM("exponential matrix \n" << expo_mat.matrix());

    return new_trans_mat;
}

// flag: body velocity or hybrid velocity
Eigen::Affine3d TransformationGenerator::getExpoMatrix(const Eigen::Affine3d& trans_mat, const std::string& flag, double delta_time)
{
    Eigen::Matrix3d EYE_3 = Eigen::MatrixXd::Identity(3,3);; // 3-by-3 identity matrix
    Eigen::Affine3d expo_mat;
    Eigen::Vector3d trans_velo;
    Eigen::Vector3d rot_omega;

    Eigen::Matrix3d skew_trans_mat = getSkewSymMatrix(trans_mat.translation());

    if (flag == "body")
    {
        velo_vec::velocityVector rand_body_velo;
        randomBodyVelocityGenerator(rand_body_velo); // initialize it by 

        velo_vec::velocityVector rand_space_velo; // define a random space velocity 

        // convert rand_body_velocity to spatial velocity
        // rand_space_velo.transV = trans_mat.linear() * rand_body_velo.transV + skew_trans_mat * trans_mat.linear() * rand_body_velo.angV;
        // rand_space_velo.angV = trans_mat.linear() * rand_body_velo.angV;

        rand_space_velo.transV = EYE_3 * rand_body_velo.transV + skew_trans_mat * EYE_3 * rand_body_velo.angV;
        rand_space_velo.angV = EYE_3 * rand_body_velo.angV;

        trans_velo = delta_time * rand_space_velo.transV;
        rot_omega= delta_time * rand_space_velo.angV;
    }
    else if (flag == "hybrid")
    {   
        velo_vec::velocityVector fixed_hybrid_velo;
        hybridVelocityGenerator(fixed_hybrid_velo);

        velo_vec::velocityVector fixed_space_velo; // define a fixed space velocity

        // convert rand_body_velocity to spatial velocity
        fixed_space_velo.transV = EYE_3 * fixed_hybrid_velo.transV + skew_trans_mat * EYE_3 * fixed_hybrid_velo.angV;
        fixed_space_velo.angV = EYE_3 * fixed_hybrid_velo.angV;

        trans_velo = delta_time * fixed_space_velo.transV;

        rot_omega = delta_time * fixed_space_velo.angV;
    }
    
    // if rot_omega near to zero then assume the expo_mat linear part is 3-by-3 identity matrix
    // ROS_INFO_STREAM("rot norm" << rot_omega.norm());
    if (rot_omega.norm() < 10e-6)
    {
        expo_mat.linear() = EYE_3; // set linear part as 3-by-3 matrix
        expo_mat.translation() = trans_velo;
    }
    else
    {
        double theta = rot_omega.norm();
        trans_velo /= theta;
        rot_omega /= theta;

        Eigen::Matrix3d skew_rot_omega = getSkewSymMatrix(rot_omega);
        
        expo_mat.linear() = EYE_3 + skew_rot_omega * sin(theta) + 
                            skew_rot_omega * skew_rot_omega * (1 - cos(theta));
        expo_mat.translation() = (EYE_3 - expo_mat.linear()) * rot_omega.cross(trans_velo) +
                                rot_omega * rot_omega.transpose() * trans_velo * theta;
    }
    return expo_mat;
}

Eigen::Matrix3d TransformationGenerator::getSkewSymMatrix(const Eigen::Vector3d& vector_in)
{
    Eigen::Matrix3d t_hat;
    t_hat << 0, -vector_in(2), vector_in(1),
        vector_in(2), 0, -vector_in(0),
        -vector_in(1), vector_in(0), 0;

    return t_hat;
}


void TransformationGenerator::randomBodyVelocityGenerator(velo_vec::velocityVector& rand_body_velo)
{

    Eigen::Vector3d trans_velo;
    Eigen::Vector3d rot_omega;

    double mean_t = 0.0; // translation mean
    double deviation_t = 0.01; // translation deviation
    double mean_r = 0.0; // rotation mean
    double deviation_r = 0.1; // rotation deviation

    // generate random body velocity translation part given specified mean and deviation
    trans_velo(0) = gaussRandNumGenerator(mean_t, deviation_t);
    trans_velo(1) = gaussRandNumGenerator(0, 0);
    trans_velo(2) = gaussRandNumGenerator(0, 0);
    
    // generate random body velocity rotation part given specified mean and deviation
    rot_omega(0) = gaussRandNumGenerator(0, 0);
    rot_omega(1) = gaussRandNumGenerator(0, 0);
    rot_omega(2) = gaussRandNumGenerator(0, 0); 

    rand_body_velo.transV = trans_velo;
    rand_body_velo.angV = rot_omega;
}

void TransformationGenerator::hybridVelocityGenerator(velo_vec::velocityVector& hybrid_velo)
{   

    Eigen::Vector3d trans_velo(0.001, 0, 0);
    Eigen::Vector3d rot_omega(0, 0, 0);

    hybrid_velo.transV = trans_velo;
    hybrid_velo.angV = rot_omega;
}

double TransformationGenerator::gaussRandNumGenerator(double mean, double std_deviation)
{
    static double V1, V2, S;
    static int phase = 0;
    double X;
     
    if ( phase == 0 ) {
        do {
            double U1 = (double)rand() / RAND_MAX;
            double U2 = (double)rand() / RAND_MAX;
             
            V1 = 2 * U1 - 1;
            V2 = 2 * U2 - 1;
            S = V1 * V1 + V2 * V2;
        } while(S >= 1 || S == 0);
         
        X = V1 * sqrt(-2 * log(S) / S);
    } else
        X = V2 * sqrt(-2 * log(S) / S);
         
    phase = 1 - phase;
 
    X = X * std_deviation + mean;
    return X;
}