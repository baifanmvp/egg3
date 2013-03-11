#include "./soEgg.h"



PRIVATE WBOOL soEgg_add_field_index_with_int32(HFIELDINDEX hFieldIndex,
                                               HFIELD hField,
                                               did_t id);

PRIVATE WBOOL soEgg_add_field_index_with_str(HFIELDINDEX hFieldIndex,
                                             HFIELD hField,
                                             did_t id);


PUBLIC soEgg* soEgg_new(const path_t* pMainDir)
{
    soEgg* pSoEgg = (soEgg*)malloc(sizeof(soEgg));
    if (!pSoEgg)
    {
        perror("[*ERROR]-soEgg_new malloc");
        exit(-1);
    }
    memset(pSoEgg, 0, sizeof(soEgg));
    
    length_t n_dir_len = strlen(pMainDir);
    pSoEgg->m_pDir = (path_t*)malloc(n_dir_len + 8);
    memcpy(pSoEgg->m_pDir, pMainDir, n_dir_len);
    *(pSoEgg->m_pDir + n_dir_len) = '\0';

    soEgg_deploy(pSoEgg);
    
    return pSoEgg;
}

PUBLIC WBOOL soEgg_delete(soEgg* pSoEgg)
{
    w_return_val_if_fail(soEgg_is_object(pSoEgg), W_FALSE);

    egg_IndexWrite_close(pSoEgg->m_hIndexWriter);
    FieldIndex_delete(pSoEgg->m_hFieldIndex);
    
    return W_TRUE;
}

PUBLIC WBOOL soEgg_deploy(soEgg* pSoEgg)
{
    w_return_val_if_fail(soEgg_is_object(pSoEgg), W_FALSE);

    if (access(pSoEgg->m_pDir, F_OK) == -1)
    {
        mkdir(pSoEgg->m_pDir, 0777);
    }
    
    HDIRECTORY lp_directory =  egg_Directory_open(filepath);
    ImLexAnalyzer* p_la = (ImLexAnalyzer*)ImCnLexAnalyzer_new();
    
    pSoEgg->m_hIndexWriter = egg_IndexWrite_open(lp_directory,  p_la, CACHE);
    pSoEgg->m_hFieldIndex = FieldIndex_new(lp_directory);

    return W_TRUE;
}

PUBLIC soBuildID soEgg_add_build(soEgg* pSoEgg, soBuild* pSoBuild)
{
    w_return_val_if_fail(soEgg_is_object(pSoEgg), -1);
    w_return_val_if_fail(soBuild_is_object(pSoBuild), -1);

    eggDocument* p_egg_document = soBuild_serialize(pSoBuild);

    HINDEXWRITE h_index_writer = pSoBuild->m_hIndexWriter;
    egg_IndexWrite_add_document(h_index_writer, p_egg_document);

    
    HFIELD h_field_iter = W_NULL;
    while ((h_field_iter = eggDocument_get_field_by_iter(p_egg_document, h_field_iter)) != W_NULL)
    {
        type_t field_type = egg_Field_get_type(h_field_iter);

        if (field_type & ORDER_BY_NUMBER)
        {
            soEgg_add_field_index_with_int32(pSoEgg->m_hFieldIndex,
                                             h_field_iter,
                                             p_egg_document->id);
        }
        else if (field_type & ORDER_BY_ALPHA)
        {
            soEgg_add_field_index_with_str(pSoEgg->m_hFieldIndex,
                                           h_field_iter,
                                           p_egg_document->id);
        }
        
    }
    
    return p_egg_document->id;
}


PUBLIC HTOPCOLLECTOR soEgg_query_build_ids(soEgg* pSoEgg, const HMULTIQUERY hMultiQuery)
{
    w_return_val_if_fail(soEgg_is_object(pSoEgg), W_NULL);
    w_return_val_if_fail(eggMultiQuery_is_object(hMultiQuery), W_NULL);

    HINDEXREADER p_index_reader = egg_IndexWrite_get_reader(pSoEgg->m_hIndexWriter);
    if (!p_index_reader)
    {
        printf("[*ERROR]-soEgg_query_build_id: IndexReader is not object\n");
        return W_NULL;
    }

    HINDEXSEARCHER p_index_searcher = egg_IndexSearcher_new(p_index_reader);
    if (!p_index_searcher)
    {
        printf("[*ERROR]-soEgg_query_build_id: IndexSearcher is not object\n");
        return W_NULL;
    }
    
    HTOPCOLLECTOR hTopCollector = egg_TopCollector_new(0, W_TRUE);
    WBOOL w_ret = egg_IndexSearcher_search_with_query(p_index_searcher,
                                                      hTopCollector,
                                                      hMultiQuery);
    
    if (w_ret == W_FALSE)
    {
        printf("[*ERROR]-soEgg_query_build_id: no result\n");

        egg_IndexSearcher_delete(p_index_searcher);
        egg_TopCollector_delete(hTopCollector);
        return W_NULL;
    }

    egg_IndexSearcher_delete(p_index_searcher);
    
    return hTopCollector;
}

PUBLIC HTOPCOLLECTOR soEgg_query_by_range(soEgg* pSoEgg,
                                          const field_t* pFieldName,
                                          FieldRangeFilter pFieldRangeFilter)
{
    w_return_val_if_fail(soEgg_is_object(pSoEgg), W_NULL);
    w_return_val_if_fail(pFieldName, W_NULL);
    w_return_val_if_fail(FieldRangeFilter_is_object(pFieldRangeFilter));

    HFIELDINDEX p_field_index = pSoEgg->m_hFieldIndex;
    TOPCOLLECTOR* pTopCollector = egg_TopCollector_new(0, EGG_TRUE);

    FieldIndex_query_by_range(p_field_index,
                              pFieldName,
                              pFieldRangeFilter,
                              pTopCollector);

    if (w_ret == W_FALSE)
    {
        printf("[*ERROR]-soEgg_query_by_range: no result\n");

        egg_TopCollector_delete(hTopCollector);
        return W_NULL;
    }

    return pTopCollector;
}



PRIVATE WBOOL soEgg_add_field_index_with_int32(HFIELDINDEX hFieldIndex,
                                               HFIELD hField,
                                               did_t id)
{
    FIELDINDEX* lp_field_index = hFieldIndex;
    FIELD* lp_field_iter = hField;
    size32_t n_field_size = 0;
    type_t field_type = egg_Field_get_type(lp_field_iter);
    const echar*  lp_field_value = egg_Field_get_value(lp_field_iter, &n_field_size);
    const echar* lp_field_name = egg_Field_get_name(lp_field_iter);

    
    if (!FieldIndex_index_is_exist(lp_field_index, lp_field_name))
    {
        FieldIndexConfig* lp_config = FieldIndexConfig_new();
        FieldIndexConfig_set_property_type(lp_config, EGG_PROP_INT32);
        FieldIndex_create_index(lp_field_index, lp_field_name, lp_config);
        FieldIndexConfig_delete(lp_config);
    }
    
    eggInt32Property* lp_int32_property = eggInt32Property_new();
    eggInt32Property_set(lp_int32_property, lp_field_name, *((int*)lp_field_value));
    
    FieldIndex_add_in_index (lp_field_index, (eggProperty*)lp_int32_property, id);
    eggProperty_delete((eggProperty*)lp_int32_property);

    return W_TRUE;
}

PRIVATE WBOOL soEgg_add_field_index_with_str(HFIELDINDEX hFieldIndex,
                                             HFIELD hField,
                                             did_t id)
{
    FIELDINDEX* lp_field_index = hFieldIndex;
    FIELD* lp_field_iter = hField;
    size32_t n_field_size = 0;
    type_t field_type = egg_Field_get_type(lp_field_iter);
    const echar*  lp_field_value = egg_Field_get_value(lp_field_iter, &n_field_size);
    const echar* lp_field_name = egg_Field_get_name(lp_field_iter);

    
    if (!FieldIndex_index_is_exist(lp_field_index, lp_field_name))
    {
        FieldIndexConfig* lp_config = FieldIndexConfig_new();
        FieldIndexConfig_set_property_type(lp_config, EGG_PROP_VARSTR);
        FieldIndex_create_index(lp_field_index, lp_field_name, lp_config);
        FieldIndexConfig_delete(lp_config);
    }
    
    eggVarStrProperty* lp_str_property = eggVarStrProperty_new();
    eggVarStrProperty_set(lp_str_property, lp_field_name, lp_field_value, n_field_size);
        
    FieldIndex_add_in_index (lp_field_index, (eggProperty*)lp_str_property, id);
    eggProperty_delete((eggProperty*)lp_str_property);

    return W_TRUE;
}


