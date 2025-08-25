package com.phantomac;

import org.bukkit.plugin.java.JavaPlugin;

public final class PhantomAC extends JavaPlugin {

    @Override
    public void onEnable() {
        // Plugin startup logic
        getLogger().info("PhantomAC has been enabled! Ready to catch cheaters.");
    }

    @Override
    public void onDisable() {
        // Plugin shutdown logic
        getLogger().info("PhantomAC has been disabled.");
    }
}
