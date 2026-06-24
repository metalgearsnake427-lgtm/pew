import { Router } from "express";
import { db, contactSubmissionsTable } from "@workspace/db";
import { SubmitContactBody } from "@workspace/api-zod";

const router = Router();

router.post("/contact", async (req, res) => {
  try {
    const parsed = SubmitContactBody.safeParse(req.body);
    if (!parsed.success) {
      return res.status(400).json({ error: "Invalid request body", details: parsed.error.issues });
    }
    const { name, email, message, projectType, phone, budget } = parsed.data;
    const [submission] = await db.insert(contactSubmissionsTable).values({
      name,
      email,
      message,
      projectType,
      phone: phone ?? null,
      budget: budget ?? null,
    }).returning();
    res.status(201).json({ id: submission.id, createdAt: submission.createdAt.toISOString() });
  } catch (err) {
    req.log.error({ err }, "Failed to submit contact");
    res.status(500).json({ error: "Failed to submit inquiry" });
  }
});

export default router;
