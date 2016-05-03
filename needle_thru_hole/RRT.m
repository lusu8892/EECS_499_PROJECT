function [ tree ] = RRT( initial_config, goal_config, max_iter, step_size, map_info)
    
    tree = struct('nodeIndex',[],'nodeConfig',[],'parentNodeIndex',[]);
    tree(1).nodeIndex = 1;
    tree(1).nodeConfig = initial_config;
    tree(1).parentNodeIndex = 1;
    
    x_resolution = 0.25;
    y_resolution = 0.25;
    theta_resolution = 0.25;
    resolution = [x_resolution y_resolution theta_resolution];
    
    plotTree();
    
    s = rng;
    
    for i = 1:max_iter
        rnd_num = rand();
        if (rnd_num < 0.05)
            % 5% chance to sample goal_config
            q_rand = goal_config;
        else
            % 95% chance to get other collision free random sample
            q_rand = collisionFreeRandomConfig(map_info);
        end
        
        tree = extendRRT(tree, q_rand, step_size, map_info, resolution);
        num_of_nodes_on_tree = length(tree.nodeConfig);
        x = tree.nodeConfig(num_of_nodes_on_tree).position(1);
        y = tree.nodeConfig(num_of_nodes_on_tree).position(2);
        plot(x,y,'o');
        pause(.01);
     end
end

%% subfunction plot nodes
function plotTree()
    clf;
    axis([0 100 0 100]);
    axis square;
    % Parameters
    tissue = [0  0  100 100 0; 0 100 100 0 0];
    wall_top = [45 45 55 55;100 55 55 100];
    wall_bottom = [45 45 55 55;0 45 45 0];
    % Plot the box.
    line(tissue(1,:),tissue(2,:),'color','blue');
    line(wall_top(1,:),wall_top(2,:),'color','blue');
    line(wall_bottom(1,:),wall_bottom(2,:),'color','blue');
    hold on;
end
