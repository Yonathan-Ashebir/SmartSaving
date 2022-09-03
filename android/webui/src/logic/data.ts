//flow constants

//component constants
//getters
export function getGroupsData() {
    try {
        let data = JSON.parse(window.preferences.getString("groupsData"))
        if (data) return data; else return {}
    }
    catch (e) { return {} }

}
export function addGroup(groupName:String){
    let data = getGroupsData();
    if(!data[groupName])data[groupName]=[];
    window.preferences.setString("groupsData",data)
}
export function addDevice(groupName:String,device:any):void{
    let data = getGroupsData()
    if(!data[groupName])data[groupName]=[];
    window.preferences.setString("groupsData",data)
}
//setters
//resources
window.yonstore = { groupsData: "{\"living room\":[{\"name\":\"lights\",\"interface\":1},{\"name\":\"tv\",\"interface\":2}],\"kitchen\":[{\"name\":\"stove\",\"interface\":3},{\"name\":\"lights\",\"interface\":4},{\"name\":\"fridge\",\"interface\":5},{\"name\":\"dish washer\",\"interface\":6},{\"name\":\"microwave\",\"interface\":7}],\"bath room\":[{\"name\":\"heater\",\"interface\":8},{\"name\":\"lights\",\"interface\":9}],\"stairs\":[{\"name\":\"head lights\",\"interface\":10},{\"name\":\"corner lights\",\"interface\":11}],\"compound\":[{\"name\":\"lights\",\"interface\":11},{\"name\":\"electric fence\",\"interface\":12}]}" }

window.preferences = { setString: (k, v) => { yonstore[k] = v }, getInteger: (k) => yonstore[k], setInteger: (k, v) => { yonstore[k] = v }, setLong: (k, v) => { yonstore[k] = v }, setFloat: (k, v) => { yonstore[k] = v }, setBoolean: (k, v) => { yonstore[k] = v }, getString: (k) => yonstore[k], getLong: () => yonstore[k], getFloat: () => yonstore[k], getBoolean: () => yonstore[k] }
window.bridge = { getField: () => { }, invokeMethod: () => { } }
window.activity = { finish: () => { } }
class Preferences {
    target
    constructor(nativeTarget) {
        if (!nativeTarget) throw new Error("> Illegal arguments Error.")
        this.target = nativeTarget;
    }

    getString(k, otherwise = null) {
        if (!k) return otherwise;
        if (typeof k !== "string") throw new Error("> Preference key can only be a string.")
        try {
            if (!this.target.getString) {
                console.warn("> Unimplemented method");
                return otherwise;
            }
            let result = this.target.getString(k)
            if (typeof result === "string" || result === null) return result; else return otherwise;
        } catch (e) {
            console.error(e);
            return otherwise;
        }
    }

    getBoolean(k, otherwise = null) {
        if (!k) return otherwise;
        if (typeof k !== "string") throw new Error("> Preference key can only be a string.")
        try {
            if (!this.target.getBoolean) {
                console.warn("> Unimplemented method");
                return otherwise;
            }
            let result = this.target.getBoolean(k)
            if (typeof result === "boolean" || result === null) return result; else return otherwise;
        } catch (e) {
            console.error(e);
            return otherwise;
        }
    }

    getInteger(k, otherwise = null) {
        if (!k) return otherwise;
        if (typeof k !== "string") throw new Error("> Preference key can only be a string.")
        try {
            if (!this.target.getInteger) {
                console.warn("> Unimplemented method");
                return otherwise;
            }
            let result = this.target.getInteger(k)
            if (typeof result === "number" || result === null) return result; else return otherwise;
        } catch (e) {
            console.error(e);
            return otherwise;
        }
    }

    getLong(k, otherwise = null) {
        if (!k) return otherwise;
        if (typeof k !== "string") throw new Error("> Preference key can only be a string.")
        try {
            if (!this.target.getLong) {
                console.warn("> Unimplemented method");
                return otherwise;
            }
            let result = this.target.getLong(k)
            if (typeof result === "number" || result === null) return result; else return otherwise;
        } catch (e) {
            console.error(e);
            return otherwise;
        }
    }

    getFloat(k, otherwise = null) {
        if (!k) return otherwise;
        if (typeof k !== "string") throw new Error("> Preference key can only be a string.")
        try {
            if (!this.target.getFloat) {
                console.warn("> Unimplemented method");
                return otherwise;
            }
            let result = this.target.getFloat(k)
            if (typeof result === "number" || result === null) return result; else return otherwise;
        } catch (e) {
            console.error(e);
            return otherwise;
        }
    }

    setString(k, v) {
        if (typeof k !== "string") throw new Error("> Preference key can not be null")
        if (typeof v !== "string") throw new Error("> Invalid value supplied")
        try {
            if (this.target.setString) {
                this.target.setString(k, v);
                return true
            }
            console.warn("> Unimplemented method")
            return false;
        } catch (e) {
            console.error(e);
            return false
        }
    }

    setBoolean(k, v) {
        if (typeof k !== "string") throw new Error("> Preference key can not be null")
        if (typeof v !== "boolean") throw new Error("> Invalid value supplied")
        try {
            if (this.target.setBoolean) {
                this.target.setBoolean(k, v);
                return true
            }
            console.warn("> Unimplemented method")
            return false;
        } catch (e) {
            console.error(e);
            return false
        }
    }

    setInteger(k, v) {
        if (typeof k !== "string") throw new Error("> Preference key can not be null")
        if (typeof v !== "number") throw new Error("> Invalid value supplied")
        try {
            if (this.target.setInteger) {
                this.target.setInteger(k, v);
                return true
            }
            console.warn("> Unimplemented method")
            return false;
        } catch (e) {
            console.error(e);
            return false
        }
    }

    setLong(k, v) {
        if (typeof k !== "string") throw new Error("> Preference key can not be null")
        if (typeof v !== "number") throw new Error("> Invalid value supplied")
        try {
            if (this.target.setLong) {
                this.target.setLong(k, v);
                return true
            }
            console.warn("> Unimplemented method")
            return false;
        } catch (e) {
            console.error(e);
            return false
        }
    }

    setFloat(k, v) {
        if (typeof k !== "string") throw new Error("> Preference key can not be null")
        if (typeof v !== "number") throw new Error("> Invalid value supplied")
        try {
            if (this.target.setFloat) {
                this.target.setFloat(k, v);
                return true
            }
            console.warn("> Unimplemented method")
            return false;
        } catch (e) {
            console.error(e);
            return false
        }
    }

}
{
    let preferences = new Preferences(window.preferences);
    window.preferences = preferences;
}