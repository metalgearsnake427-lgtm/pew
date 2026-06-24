import { motion } from "framer-motion";
import { useListServices } from "@workspace/api-client-react";
import { Skeleton } from "@/components/ui/skeleton";
import * as Icons from "lucide-react";

export function Services() {
  const { data: services, isLoading } = useListServices();

  return (
    <section className="py-32 px-4 md:px-8 bg-card" id="services">
      <div className="max-w-7xl mx-auto">
        <motion.div
          initial={{ opacity: 0, y: 20 }}
          whileInView={{ opacity: 1, y: 0 }}
          viewport={{ once: true }}
          transition={{ duration: 0.8 }}
          className="text-center mb-20"
        >
          <h2 className="text-4xl md:text-6xl font-serif">Our Expertise</h2>
          <div className="w-24 h-[1px] bg-primary mx-auto mt-8" />
        </motion.div>

        {isLoading ? (
          <div className="grid grid-cols-1 md:grid-cols-3 gap-12">
            {[1, 2, 3].map((i) => (
              <Skeleton key={i} className="h-64 w-full" />
            ))}
          </div>
        ) : (
          <div className="grid grid-cols-1 md:grid-cols-3 gap-12">
            {services?.map((service, index) => {
              const IconComponent = (Icons as any)[service.icon] || Icons.Box;

              return (
                <motion.div
                  key={service.id}
                  initial={{ opacity: 0, y: 50 }}
                  whileInView={{ opacity: 1, y: 0 }}
                  viewport={{ once: true, margin: "-100px" }}
                  transition={{ duration: 0.8, delay: index * 0.2 }}
                  className="group relative border border-border p-8 hover:bg-background transition-colors duration-500"
                >
                  <div className="mb-6 text-primary">
                    <IconComponent strokeWidth={1} size={48} className="transition-transform duration-500 group-hover:scale-110" />
                  </div>
                  <h3 className="text-2xl font-serif mb-4">{service.title}</h3>
                  <p className="text-muted-foreground font-light leading-relaxed mb-8">
                    {service.description}
                  </p>
                  <ul className="space-y-3">
                    {service.features?.map((feature, i) => (
                      <li key={i} className="flex items-center text-sm tracking-wide text-foreground">
                        <span className="w-1.5 h-1.5 bg-primary mr-3 rounded-full" />
                        {feature}
                      </li>
                    ))}
                  </ul>
                </motion.div>
              );
            })}
          </div>
        )}
      </div>
    </section>
  );
}
