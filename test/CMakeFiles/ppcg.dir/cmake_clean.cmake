file(REMOVE_RECURSE
  "ppcg.pdb"
  "ppcg"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/ppcg.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
