import { RouterProvider } from "react-router";
import { router } from "./routes";
import { AuthProvider } from "../context/AuthContext";

export default function App() {
    return (
        <AuthProvider>
            <div className="dark">
                <RouterProvider router={router} />
            </div>
        </AuthProvider>
    );
}
