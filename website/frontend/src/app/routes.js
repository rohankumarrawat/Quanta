import { createElement } from "react";
import { createBrowserRouter } from "react-router";
import { Root } from "./components/Root";
import { AdminGuard } from "./components/AdminGuard";
import { IntroductionPage } from "./pages/IntroductionPage";
import { QuickstartPage } from "./pages/QuickstartPage";
import { CoreSyntaxPage } from "./pages/CoreSyntaxPage";
import { AboutPage } from "./pages/AboutPage";
import { CommunityPage } from "./pages/CommunityPage";
import { BlogPage } from "./pages/BlogPage";
import { BlogDetailPage } from "./pages/BlogDetailPage";
import { QuestionDetailPage } from "./pages/QuestionDetailPage";
import { TryQuantaPage } from "./pages/TryQuantaPage";
import { ProfilePage } from "./pages/ProfilePage";
import { SettingsPage } from "./pages/SettingsPage";
import { AdminLayout } from "./pages/admin/AdminLayout";
import { AdminDashboard } from "./pages/admin/AdminDashboard";
import { AdminUsers } from "./pages/admin/AdminUsers";
import { AdminQuestions } from "./pages/admin/AdminQuestions";
import { AdminBlog } from "./pages/admin/AdminBlog";
import { AdminNotifications } from "./pages/admin/AdminNotifications";

export const router = createBrowserRouter([
    {
        path: "/",
        Component: Root,
        children: [
            { index: true, Component: IntroductionPage },
            { path: "quickstart", Component: QuickstartPage },
            { path: "syntax", Component: CoreSyntaxPage },
            { path: "about", Component: AboutPage },
            { path: "community", Component: CommunityPage },
            { path: "community/:id", Component: QuestionDetailPage },
            { path: "blog", Component: BlogPage },
            { path: "blog/:id", Component: BlogDetailPage },
            { path: "try", Component: TryQuantaPage },
            { path: "profile", Component: ProfilePage },
            { path: "settings", Component: SettingsPage },
        ],
    },
    {
        path: "/admin",
        element: createElement(AdminGuard, null, createElement(AdminLayout, null)),
        children: [
            { index: true, Component: AdminDashboard },
            { path: "users", Component: AdminUsers },
            { path: "questions", Component: AdminQuestions },
            { path: "blog", Component: AdminBlog },
            { path: "notifications", Component: AdminNotifications },
        ],
    },
]);
