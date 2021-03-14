function ijw = edgelist_generator (SCALE, edgefactor)
  
  
  %% Set number of vertices.
  N = 2^SCALE;

  %% Set number of edges.
  M = edgefactor * N;

  %% Set initiator probabilities.
  [A, B, C] = deal (0.57, 0.19, 0.19);

  %% Create index arrays.
  ijw = ones (3, M);
  %% Loop over each order of bit.
  ab = A + B;
  c_norm = C/(1 - (A + B));
  a_norm = A/(A + B);

  for ib = 1:SCALE,
    %% Compare with probabilities and set bits of indices.
    ii_bit = rand (1, M) > ab;
    jj_bit = rand (1, M) > ( c_norm * ii_bit + a_norm * not (ii_bit) );
    ijw(1:2,:) = ijw(1:2,:) + 2^(ib-1) * [ii_bit; jj_bit; ];
  end
  
  %% Generate weights
  ijw(3,:) = unifrnd(0, 1, 1, M);
  
  %% Permute vertex labels
  p = randperm (N);
  ijw(1:2,:) = p(ijw(1:2,:));

  %% Permute the edge list
  p = randperm (M);
  ijw = ijw(:, p);

  %% Adjust to zero-based labels.
  ijw(1:2,:) = ijw(1:2,:) - 1;
