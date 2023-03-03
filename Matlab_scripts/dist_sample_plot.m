% times = [202478, 202637, 202463, 202510, 202637, 202473, 202507, 202636, 202466, 202503, 202639, 202468, 202503, 202652, 202645, 202632, 202471, 202507, 202635, 202477, 202494, 202471, 202481, 202501, 202636, 202488, 202481, 202469, 202493, 202488, 202657, 202482, 202631, 202471, 202510, 202496, 202523, 202628, 202470, 202506, 202633, 202483, 202490, 202639, 202489, 202640, 202473, 202507, 202495, 202521, 202637, 202469, 202503, 202639, 202458, 202512, 202656, 202641, 202633, 202474, 202501, 202642, 202457, 202509, 202657, 202643, 202636, 202478, 202496, 202653, 202644, 202634, 202476, 202500, 202635, 202490, 202484, 202468, 202493, 202649, 202652, 202646, 202639, 202484, 202483, 202468, 202494, 202487, 202636, 202478, 202490, 202248, 202636, 202470, 202501, 202462, 202485, 202503, 202639, 202468];

% other_times = [202289, 202230, 202302, 201421, 202257, 202000, 202265, 201240, 201371, 202229, 202301, 202281, 200972, 202270, 201108, 202297, 202119, 201999, 201064, 201948, 201267, 201781, 201970, 202200, 201087, 201853, 202289, 201359, 202118, 202229, 201997, 202308, 201440, 202275, 201978, 201926, 201402, 201966, 201942, 202310, 202222, 202287, 202195, 202259, 202133, 201669, 201213, 201639, 201860, 201713, 202243, 201963, 201504, 201220, 201664, 201988, 201965, 201502, 201665, 201444, 201500, 202286, 201667, 202138, 202105, 201804, 201988, 201406, 202296, 201650, 202263, 202128, 201857, 202068, 201509, 201691, 202267, 201946, 202286, 202128, 202288, 202285, 202138, 201945, 201115, 201760, 202035, 201403, 202055, 201833, 202055, 202053, 201545, 201663, 201506, 202281, 201947, 201966, 201979, 202265];

times = [202263; 202291; 202291; 202098; 202288; 201848; 202295; 201936; 202283; 202293;];
other_times = [200312; 200291; 200301; 200309; 200284; 200289; 200280; 200307; 200314; 200309;];

scatter(1:length(times), times); hold on;
scatter(1:length(other_times), other_times); hold on;
xlabel("sample")
ylabel("time [us]")