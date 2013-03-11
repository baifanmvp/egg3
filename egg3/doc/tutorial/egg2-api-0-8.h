
/**
      \page page_egg2.0.6 egg2.0.6 Tutorial

      \section sec_tutorial_index Make Index
      \code
      
      HEGGDIRECTORY EGGAPI 	eggDirectory_open (const path_t *filepath);
    
      EBOOL EGGAPI 	eggDirectory_close (HEGGDIRECTORY hEggDirectory);

      path_t *EGGAPI 	eggDirectory_getName (HEGGDIRECTORY hEggDirectory);

      EBOOL EGGAPI 	eggDirectory_init (HEGGDIRECTORY hEggDirectory);
    
      HEGGFILE EGGAPI eggDirectory_get_file (HEGGDIRECTORY hEggDirectory, const echar *lpFileName);
*/
