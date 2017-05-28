# for i in a b c d e f g h i j
for i in v17a_1e v17b_1e v17c_1e v17c_1h v17c_1m v17c_3e v17c_3m v17d_1e v17e_1e v17f_1e v17g_1e v17h_1e v17i_1e v17j_1e v17k_1e 
do
#  echo === trd_v17${i}_1e.digi.par
#  tail -6 trd_v17${i}_1e.digi.par
  echo === trd_${i}.digi.par
  tail -6 trd_${i}.digi.par
  echo
  echo
done
