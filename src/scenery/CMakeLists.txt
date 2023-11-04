add_liar_module(scenery)

find_package(happly)
if(happly_FOUND)
    target_link_libraries(scenery
        PRIVATE
            happly::happly
    )
    target_compile_definitions(scenery
        PUBLIC
            LIAR_HAVE_HAPPLY=1
    )
endif()
