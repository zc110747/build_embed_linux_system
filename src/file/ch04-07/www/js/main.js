// @ts-nocheck

//config for axios default
//axios.defaults.baseURL='http://192.168.204.1:8080'  //用于跨域访问
axios.defaults.timeout=500

//设置硬件的模块
Vue.createApp({
    data(){
        return {
            message:"Information!",
            isShow:true,
            intervalId:null,
            deviceInfo:{
                command: 1,
                data: {
                  angle: 0,
                  ap: { als: 0, ir: 0, ps: 0 },
                  beep: 0,
                  hx711: 0,
                  icm: {
                    accel_x: 0,
                    accel_y: 0,
                    accel_z: 0,
                    gyro_x: 0,
                    gyro_y: 0,
                    gyro_z: 0,
                    temp_act: 0
                  },
                  led: 0,
                  sysinfo: {
                    cpu_info: '',
                    disk_total: 0,
                    disk_used: 0,
                    kernel_info: '',
                    ram_total: 0,
                    ram_used: 0
                  },
                  vf610_adc: 0
                }
            }
        }   
    },
    methods:{
        dev_set: function(device, action){
            axios({
                method:"post",
                url:"axiosDeviceSet.json",
                params:{
                    dev:device,
                    set:action
                }
            }).then(res=>console.log(res)).catch(err=>console.log(err));
            this.message = device+ " " + action;
        },

        changeShow: function(val){
            if (val == 0)
                this.isShow = true;
            else
                this.isShow = false;
        },

        dataRefresh: function(){
            let t = this;
            if (this.intervalId != null){
                return;
            }
            this.intervalId = setInterval(() => {
                axios({
                    method:"get",
                    url:"axiosDeviceGet.json",
                }).then(res=>{
                    console.log(res.data)
                    for (let key in res.data){
                        if (t.deviceInfo.hasOwnProperty(key)){
                            t.deviceInfo[key] = res.data[key]
                        }
                    }
                }).catch(err=>console.log(err));
            }, 1000);
        },

        clear: function(){
            clearInterval(this.intervalId);
            this.intervalId = null;
        }
    },
    created(){
        this.dataRefresh();
    },
    destroyed(){
        this.clear();
    }
}).mount("#app");
