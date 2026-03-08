import { Outlet, useLocation } from "react-router";
import { Navbar } from "./Navbar";
import { Sidebar } from "./Sidebar";

export function Root() {
    const location = useLocation();
    const hideSidebar = location.pathname === "/community" || location.pathname === "/blog";

    return (
        <div className="min-h-screen bg-background">
            <Navbar />
            <div className="flex">
                {!hideSidebar && <Sidebar />}
                <main className={`${!hideSidebar ? "ml-[260px]" : ""} mt-[72px] flex-1 min-w-0`}>
                    <div className="max-w-[1180px] mx-auto px-8 py-12">
                        <Outlet />
                    </div>
                </main>
            </div>
        </div>
    );
}
