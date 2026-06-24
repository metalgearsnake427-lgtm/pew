import { motion } from "framer-motion";
import { useState } from "react";
import { Link } from "wouter";
import { useListProjects } from "@workspace/api-client-react";
import { Skeleton } from "@/components/ui/skeleton";

const IMAGES = [
  "/projects/project1.png",
  "/projects/project2.png",
  "/projects/project3.png",
  "/projects/project4.png",
  "/projects/project5.png",
];

const CATEGORIES = ["all", "residential", "commercial", "interior", "landscape", "renovation"];

export function FeaturedProjects() {
  const { data: projects, isLoading } = useListProjects();
  const [filter, setFilter] = useState("all");

  const filteredProjects = projects?.filter(
    (p) => filter === "all" || p.category === filter
  ) || [];

  return (
    <section className="py-32 px-4 md:px-8 bg-background" id="projects">
      <div className="max-w-7xl mx-auto">
        <div className="flex flex-col md:flex-row md:items-end justify-between gap-8 mb-16">
          <motion.div
            initial={{ opacity: 0, x: -20 }}
            whileInView={{ opacity: 1, x: 0 }}
            viewport={{ once: true }}
            transition={{ duration: 0.8 }}
          >
            <h2 className="text-4xl md:text-6xl font-serif">Selected Works</h2>
            <p className="text-muted-foreground mt-4 text-lg max-w-md">
              A curated selection of our most definitive spaces.
            </p>
          </motion.div>

          <motion.div 
            initial={{ opacity: 0, x: 20 }}
            whileInView={{ opacity: 1, x: 0 }}
            viewport={{ once: true }}
            transition={{ duration: 0.8 }}
            className="flex flex-wrap gap-4"
          >
            {CATEGORIES.map((cat) => (
              <button
                key={cat}
                onClick={() => setFilter(cat)}
                className={`text-sm uppercase tracking-widest pb-1 border-b-2 transition-all duration-300 ${
                  filter === cat 
                    ? "border-primary text-primary" 
                    : "border-transparent text-muted-foreground hover:text-foreground"
                }`}
              >
                {cat}
              </button>
            ))}
          </motion.div>
        </div>

        {isLoading ? (
          <div className="grid grid-cols-1 md:grid-cols-2 gap-8">
            {[1, 2, 3, 4].map((i) => (
              <Skeleton key={i} className="aspect-[4/3] w-full" />
            ))}
          </div>
        ) : (
          <motion.div layout className="grid grid-cols-1 md:grid-cols-2 gap-8 gap-y-16">
            {filteredProjects.map((project, index) => (
              <motion.div
                layout
                initial={{ opacity: 0, y: 50 }}
                whileInView={{ opacity: 1, y: 0 }}
                viewport={{ once: true, margin: "-100px" }}
                transition={{ duration: 0.8, delay: index * 0.1 }}
                key={project.id}
                className="group relative cursor-pointer"
              >
                <Link href={`/project/${project.id}`}>
                  <div className="overflow-hidden bg-card aspect-[4/3] relative">
                    <img 
                      src={IMAGES[project.id % IMAGES.length]} 
                      alt={project.title}
                      className="w-full h-full object-cover transition-transform duration-700 group-hover:scale-105"
                    />
                    <div className="absolute inset-0 bg-black/0 group-hover:bg-black/20 transition-colors duration-500" />
                  </div>
                  <div className="mt-6 flex justify-between items-start">
                    <div>
                      <h3 className="text-2xl font-serif group-hover:text-primary transition-colors">{project.title}</h3>
                      <p className="text-muted-foreground capitalize tracking-wider text-sm mt-2">{project.category} &mdash; {project.year}</p>
                    </div>
                    <span className="text-primary opacity-0 group-hover:opacity-100 transition-opacity duration-300 uppercase text-sm tracking-widest border-b border-primary pb-1">
                      View Project
                    </span>
                  </div>
                </Link>
              </motion.div>
            ))}
          </motion.div>
        )}
      </div>
    </section>
  );
}
