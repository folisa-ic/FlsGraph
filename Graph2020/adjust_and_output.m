function ijw = adjust_and_output (ijw)
%% Compute a sparse adjacency matrix representation
%% of the graph with edges from ijw.

  %% Remove self-edges.
  ijw(:, ijw(1,:) == ijw(2,:)) = [];
  
  num_edge =  size (ijw, 2);
  %% Adjust away from zero labels.
  ijw(1:2,:) = ijw(1:2,:) + 1;
  
  test = fopen('data.txt','wt');
  for i=1:num_edge
    fprintf(test,'%d %d %f\n',ijw(1,i)-1 ,ijw(2,i)-1 ,ijw(3,i)); 
  end
  
  fclose(test);
  fprintf('finish the graph construction!\n')