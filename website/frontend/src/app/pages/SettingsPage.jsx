import { useAuth } from "../../context/AuthContext";
import { Bell, Shield, Moon, Monitor, Key } from "lucide-react";

export function SettingsPage() {
    const { user } = useAuth();

    if (!user) {
        return (
            <div className="flex items-center justify-center min-h-[50vh]">
                <p className="text-muted-foreground">Please log in to view settings.</p>
            </div>
        );
    }

    return (
        <div className="max-w-3xl mx-auto space-y-8">
            <div className="space-y-4">
                <h1 className="text-4xl font-bold">Settings</h1>
                <p className="text-muted-foreground">
                    Manage your preferences and account security.
                </p>
            </div>

            <div className="space-y-6">
                {/* Appearance Settings */}
                <div className="bg-card border border-border rounded-xl spill-hidden">
                    <div className="px-6 py-4 border-b border-border bg-muted/20">
                        <h3 className="font-semibold text-lg flex items-center gap-2">
                            <Monitor className="w-5 h-5 text-[#22d3ee]" /> Appearance
                        </h3>
                    </div>
                    <div className="p-6 space-y-4">
                        <div className="flex items-center justify-between">
                            <div>
                                <p className="font-medium">Theme</p>
                                <p className="text-sm text-muted-foreground">Select your preferred interface theme</p>
                            </div>
                            <div className="flex bg-muted p-1 rounded-lg">
                                <button className="px-4 py-2 rounded-md bg-background shadow-sm text-sm font-medium border border-border">Dark</button>
                                <button className="px-4 py-2 rounded-md text-sm font-medium text-muted-foreground hover:text-foreground disabled:opacity-50" disabled>Light (Soon)</button>
                            </div>
                        </div>
                    </div>
                </div>

                {/* Notification Settings */}
                <div className="bg-card border border-border rounded-xl overflow-hidden">
                    <div className="px-6 py-4 border-b border-border bg-muted/20">
                        <h3 className="font-semibold text-lg flex items-center gap-2">
                            <Bell className="w-5 h-5 text-[#a855f7]" /> Notifications
                        </h3>
                    </div>
                    <div className="p-6 space-y-6">
                        <div className="flex items-center justify-between">
                            <div>
                                <p className="font-medium">Email Notifications</p>
                                <p className="text-sm text-muted-foreground">Receive emails about your questions and answers</p>
                            </div>
                            <label className="relative inline-flex items-center cursor-pointer">
                                <input type="checkbox" className="sr-only peer" defaultChecked />
                                <div className="w-11 h-6 bg-muted peer-focus:outline-none rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:border-gray-300 after:border after:rounded-full after:h-5 after:w-5 after:transition-all peer-checked:bg-[#22d3ee]"></div>
                            </label>
                        </div>
                        <div className="flex items-center justify-between">
                            <div>
                                <p className="font-medium">Community Updates</p>
                                <p className="text-sm text-muted-foreground">Occasional alerts about new Quanta features</p>
                            </div>
                            <label className="relative inline-flex items-center cursor-pointer">
                                <input type="checkbox" className="sr-only peer" defaultChecked />
                                <div className="w-11 h-6 bg-muted peer-focus:outline-none rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:border-gray-300 after:border after:rounded-full after:h-5 after:w-5 after:transition-all peer-checked:bg-[#a855f7]"></div>
                            </label>
                        </div>
                    </div>
                </div>

                {/* Security Settings */}
                <div className="bg-card border border-border rounded-xl overflow-hidden">
                    <div className="px-6 py-4 border-b border-border bg-muted/20">
                        <h3 className="font-semibold text-lg flex items-center gap-2">
                            <Shield className="w-5 h-5 text-red-400" /> Security
                        </h3>
                    </div>
                    <div className="p-6 space-y-6">
                        <div className="flex items-center justify-between">
                            <div className="flex items-center gap-3">
                                <div className="p-2 bg-muted rounded-lg">
                                    <Key className="w-5 h-5 text-muted-foreground" />
                                </div>
                                <div>
                                    <p className="font-medium">Change Password</p>
                                    <p className="text-sm text-muted-foreground">Update your account password</p>
                                </div>
                            </div>
                            <button className="px-4 py-2 bg-muted hover:bg-muted/80 text-foreground rounded-lg text-sm font-medium transition-colors" disabled>
                                Coming Soon
                            </button>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    );
}
