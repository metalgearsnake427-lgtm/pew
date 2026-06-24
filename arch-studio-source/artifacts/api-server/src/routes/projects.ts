import { Router } from "express";
import { db, projectsTable } from "@workspace/db";
import { eq, desc } from "drizzle-orm";

const router = Router();

router.get("/projects", async (req, res) => {
  try {
    const projects = await db.select().from(projectsTable).orderBy(desc(projectsTable.createdAt));
    res.json(projects);
  } catch (err) {
    req.log.error({ err }, "Failed to list projects");
    res.status(500).json({ error: "Failed to fetch projects" });
  }
});

router.get("/projects/featured", async (req, res) => {
  try {
    const projects = await db.select().from(projectsTable).where(eq(projectsTable.featured, true)).orderBy(desc(projectsTable.year));
    res.json(projects);
  } catch (err) {
    req.log.error({ err }, "Failed to list featured projects");
    res.status(500).json({ error: "Failed to fetch featured projects" });
  }
});

router.get("/projects/stats", async (req, res) => {
  try {
    const allProjects = await db.select().from(projectsTable);
    res.json({
      totalProjects: allProjects.length,
      completedProjects: allProjects.length,
      happyClients: 87,
      awardsWon: 24,
      yearsOfExperience: 15,
    });
  } catch (err) {
    req.log.error({ err }, "Failed to get project stats");
    res.status(500).json({ error: "Failed to fetch stats" });
  }
});

router.get("/projects/:id", async (req, res) => {
  try {
    const id = parseInt(req.params.id, 10);
    if (isNaN(id)) return res.status(400).json({ error: "Invalid id" });
    const [project] = await db.select().from(projectsTable).where(eq(projectsTable.id, id));
    if (!project) return res.status(404).json({ error: "Not found" });
    res.json(project);
  } catch (err) {
    req.log.error({ err }, "Failed to get project");
    res.status(500).json({ error: "Failed to fetch project" });
  }
});

export default router;
