import { motion } from "framer-motion";
import { useGetProjectStats } from "@workspace/api-client-react";

export function Stats() {
  const { data: stats, isLoading } = useGetProjectStats();

  if (isLoading || !stats) return null;

  const statItems = [
    { label: "Years of Experience", value: stats.yearsOfExperience },
    { label: "Completed Projects", value: stats.completedProjects },
    { label: "Happy Clients", value: stats.happyClients },
    { label: "Awards Won", value: stats.awardsWon },
  ];

  return (
    <section className="py-24 bg-foreground text-background">
      <div className="max-w-7xl mx-auto px-4 md:px-8">
        <div className="grid grid-cols-2 md:grid-cols-4 gap-8 md:gap-12 divide-x divide-background/10">
          {statItems.map((stat, index) => (
            <motion.div
              key={stat.label}
              initial={{ opacity: 0, y: 20 }}
              whileInView={{ opacity: 1, y: 0 }}
              viewport={{ once: true }}
              transition={{ duration: 0.8, delay: index * 0.1 }}
              className="text-center pl-4 md:pl-0"
            >
              <div className="text-4xl md:text-6xl font-serif text-primary mb-2">
                {stat.value}
                <span className="text-primary/60">+</span>
              </div>
              <div className="text-sm tracking-widest uppercase text-background/60">
                {stat.label}
              </div>
            </motion.div>
          ))}
        </div>
      </div>
    </section>
  );
}
