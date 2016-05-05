function [ tree ] = RRT( initial_config, goal_config, max_iter, step_size, map_info)
    
    tree = struct('nodeIndex',[],'nodeConfig',[],'parentNodeIndex',[]);
    tree.nodeIndex(1,:) = 1;
    tree.nodeConfig(:,1) = initial_config;
    tree.parentNodeIndex(1,:) = 1;
    
    x_resolution = 1;
    y_resolution = 1;
    theta_resolution = 0.1 * pi;
    resolution = [x_resolution y_resolution theta_resolution]';
    
    plotTree();
    plot(initial_config(1),initial_config(2),'.','LineWidth',0.4, 'color','black');
%     s = rng;
    
    for i = 1:max_iter
        rnd_num = rand();
        if (rnd_num < 0.05)
            % less than 5% chance to sample goal_config
            q_rand = goal_config;
        else
            % more than 95% chance to get other collision free random sample
            q_rand = collisionFreeRandomConfig(map_info);
        end
        
        tree = extendRRT(tree, q_rand, step_size, map_info, resolution);
        num_of_nodes_on_tree = length(tree.nodeIndex);
        
        child_node_pose = tree.nodeConfig(:,num_of_nodes_on_tree);
        parent_node_index = tree.parentNodeIndex(num_of_nodes_on_tree);
        parent_node_pose = tree.nodeConfig(:,parent_node_index);
        
        plot(child_node_pose(1),child_node_pose(2),'.','LineWidth',0.4, 'color','black');
        
        A = [parent_node_pose(1:2) child_node_pose(1:2)];
        
        line(A(1,:), A(2,:), 'color','red');
        
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
