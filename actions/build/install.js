//Install Qt 5.12, Discord RPC and libentertaining

const core = require('@actions/core');
const exec = require('@actions/exec');
const process = require('process');

(async () => {
    if (process.platform == 'win32') {
        //TODO
        core.setFailed("Not running on a supported platform.");
        return;
    }
    
    //Install Discord RPC
    {
        let options = {
            cwd: `${process.cwd()}/discord-rpc`
        };
        
        let installPrefix;
        await exec.exec("git clone https://github.com/discordapp/discord-rpc.git");
        if (process.platform == 'linux') {
            installPrefix = "/usr"
        } else {
            installPrefix = "/usr/local"
        }
        await exec.exec(`cmake . -DCMAKE_INSTALL_PREFIX=${installPrefix} -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_LIBDIR=lib -DBUILD_EXAMPLES=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON`, [], options);
        await exec.exec(`make`, [], options);
        await exec.exec(`sudo make install`, [], options);
    }
    
    //Install libentertaining
    {
        let options = {
            cwd: `${process.cwd()}/libentertaining`
        };
        
        await exec.exec("git clone https://github.com/vicr123/libentertaining.git");
        await exec.exec(`git checkout ${core.getInput("libentertaining-branch")}`, [], options);
        await exec.exec(`qmake`, [], options);
        await exec.exec(`make`, [], options);
        await exec.exec(`sudo make install INSTALL_ROOT=/`, [], options);
    }
})();
