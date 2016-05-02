%% collisionFreeRandomConfig.m
% This function is used to check if needle semicircle curve intersected
% with the straight line e.g., the wall or object
% Input: 1. the straight to check
%        2. the needle struc
%		 3. the transformation matrix from needle to tissue
% 
% Output: boolean value which represents collided or not.
%         false: no intersection with semicircle curve
%         true: intersected with semicircle curve

%% main function
function [ sample_node ] = collisionFreeRandomConfig(rnd_num, map_info)
    sample_node.position = [];
    sample_node.direction = 0;

    while (true)
       
        % based on map info generate random sample configuration and check collision
        sample_node.position(1,:) = (map_info(2).start(1) - map_info(4).start(1)) * rnd_num;
        sample_node.position(2,:) = (map_info(1).start(2) - map_info(3).start(2)) * rnd_num;
        sample_node.position(3,:) = 0;
        sample_node.direction = (2 * pi) * rnd_num;
        
        collision = collisionDetection(map_info, sample_node);
        
        if (collision == true)
            continue;
        else
            return;
        end
    end
end