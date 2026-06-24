import { Router } from "express";
import { db, testimonialsTable } from "@workspace/db";

const router = Router();

router.get("/testimonials", async (req, res) => {
  try {
    const testimonials = await db.select().from(testimonialsTable);
    res.json(testimonials);
  } catch (err) {
    req.log.error({ err }, "Failed to list testimonials");
    res.status(500).json({ error: "Failed to fetch testimonials" });
  }
});

export default router;
