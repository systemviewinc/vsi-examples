1. Create vck example platform
    - compile platform.
2. Create a system
2.1 AI-e context:
    Place software wizard into AI-e context
        -- name : "img_pracessing"
        -- source directory : "aie_src"
        -- C++ Function :  "cardano::graph kernel_graph"
2.2 Software context:
    Place software wizard into AI-e context
        -- source directory : "cpu_src"
        -- C++ Function :  "streams_to_img"
        -- Access type :  "Memory"
        -- name : "streams_to_img_soft"
    Place software wizard into AI-e context
        -- C++ Function :  "img_to_streams"
        -- source directory : "cpu_src"
        -- Access type :  "Memory"
        -- name : "img_to_streams_soft"
2.3 Hardware Context:
    Place RTL IP: vsi:ip:aie_to_aximm:1.0
        - streams : 1
        - name: "aie_to_ps"
    Place RTL IP: vsi:ip:aximm_to_aie:1.0
        - streams : 1
        - name: "ps_to_aie"
3 Connect IP's:
    img_to_streams_soft -> ps_to_ai -> img_pracessing -> aie_to_ps -> streams_to_img_soft


