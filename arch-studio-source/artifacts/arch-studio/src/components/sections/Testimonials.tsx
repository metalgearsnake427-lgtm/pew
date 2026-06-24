import { motion } from "framer-motion";
import { useListTestimonials } from "@workspace/api-client-react";
import { Skeleton } from "@/components/ui/skeleton";
import { Quote } from "lucide-react";

export function Testimonials() {
  const { data: testimonials, isLoading } = useListTestimonials();

  return (
    <section className="py-32 px-4 md:px-8 bg-background relative overflow-hidden">
      <div className="absolute top-0 left-0 w-1/3 h-full bg-card/50 -z-10 skew-x-12 transform -translate-x-1/2" />
      
      <div className="max-w-7xl mx-auto">
        <motion.div
          initial={{ opacity: 0, x: -50 }}
          whileInView={{ opacity: 1, x: 0 }}
          viewport={{ once: true }}
          transition={{ duration: 0.8 }}
          className="mb-16 md:mb-24"
        >
          <h2 className="text-4xl md:text-6xl font-serif">Client Words</h2>
        </motion.div>

        {isLoading ? (
          <div className="grid grid-cols-1 md:grid-cols-2 gap-12">
            {[1, 2].map((i) => (
              <Skeleton key={i} className="h-64 w-full" />
            ))}
          </div>
        ) : (
          <div className="grid grid-cols-1 md:grid-cols-2 gap-12 lg:gap-20">
            {testimonials?.map((testimonial, index) => (
              <motion.div
                key={testimonial.id}
                initial={{ opacity: 0, y: 30 }}
                whileInView={{ opacity: 1, y: 0 }}
                viewport={{ once: true }}
                transition={{ duration: 0.8, delay: index * 0.2 }}
                className="relative"
              >
                <Quote className="absolute -top-6 -left-6 text-card w-16 h-16 -z-10" />
                <p className="text-xl md:text-2xl font-serif italic leading-relaxed text-foreground/90 mb-8">
                  "{testimonial.quote}"
                </p>
                <div className="flex items-center gap-4">
                  <div className="w-12 h-12 rounded-full bg-muted overflow-hidden flex items-center justify-center font-serif text-lg text-muted-foreground border border-border">
                    {testimonial.avatarUrl ? (
                      <img src={testimonial.avatarUrl} alt={testimonial.name} className="w-full h-full object-cover" />
                    ) : (
                      testimonial.name.charAt(0)
                    )}
                  </div>
                  <div>
                    <h4 className="font-semibold uppercase tracking-wider text-sm">{testimonial.name}</h4>
                    <p className="text-primary text-xs tracking-widest">{testimonial.role}, {testimonial.company}</p>
                  </div>
                </div>
              </motion.div>
            ))}
          </div>
        )}
      </div>
    </section>
  );
}
