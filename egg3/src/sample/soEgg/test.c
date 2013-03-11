#Include "soEgg.h"


int soEgg_import();
int soEgg_query();


int main()
{
    return 1;
}


int soEgg_import()
{
    soEgg* pEgg = soEgg_new("./soEgg_data/");

    FILE* fd = fopen("./soEgg.txt", "r+");
    while (!feof(fd))
    {
        soBuild* p_build = soBuild_new(W_NULL, W_NULL);
        
        char buf[1024] = {0};
        int i = 0;
        for (i = 0; i < 6; i++)
        {
            fgets(buf, 1024, fd);
            *(buf + strlen(buf) - 2) = '\0';
            switch (i)
            {
            case 0:
                soBuild_set_name(p_build, buf);
                break;
            case 1:
                soBuild_set_sname(p_build, buf);
                break;
            case 2:
                soBuild_set_address(p_build, buf);
                break;
            case 3:
                soBuild_set_area(p_build, buf);
                break;
            case 4:
                soBuild_set_size(p_build, buf);
                break;
            case 5:
                soBuild_set_price(p_build, buf);
                break;
            default:
                break;
            }
        }

        soBuildID id = soEgg_add_build(pEgg, p_build);
        
        soBuild_delete(p_build);
    }
        
    fclose(fd);
    
    soEgg_delete(pEgg);

    return 1;
}


int soEgg_query()
{
    soEgg* pEgg = soEgg_new("./soEgg_data/");

    ImLexAnalyzer* p_la = (ImLexAnalyzer*)ImCnLexAnalyzer_new();
    HQUERY h_name_query = egg_Query_new_string(p_la, BUILD_NAME_FIELD, "出租", W_FALSE);

    HMULTIQUERY hMultiQuery = egg_MultiQuery_new();
    
    HTOPCOLLECTOR p_top_collector = soEgg_query_build_ids(pEgg, hMultiQuery);

    ImCnLexAnalyzer_delete(p_la);
    
    soEgg_delete(pEgg);

    return 1;
}

